//----------------------------------------------------------------------------------------------------------------------
//	CSandboxProfile.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSandboxProfile.h"

#include "CFileDataSource.h"
#include "CFolder.h"

#include <regex.h>
#include <unistd.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	const	CString	sProfilesFolderPath(OSSTR("/System/Library/Sandbox/Profiles"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSandboxProfile::Internals

class CSandboxProfile::Internals {
	public:
		struct Node {
			public:
				enum Kind {
					kKindList,
					kKindString,
					kKindSymbol,
				};

			public:
						Node(Kind kind, const CString& string) : mKind(kind), mString(string) {}
						Node(const TArray<Node>& nodes) : mKind(kKindList), mNodes(nodes) {}

				bool	isSymbol(const CString& string) const
							{ return (mKind == kKindSymbol) && (mString == string); }
				bool	isList(const CString& symbol) const
							{ return (mKind == kKindList) && !mNodes.isEmpty() && mNodes[0].isSymbol(symbol); }

			public:
				Kind			mKind;
				CString			mString;
				TNArray<Node>	mNodes;
		};

	public:
		struct Value {
			public:
				enum Kind {
					kKindFalse,
					kKindStrings,
					kKindUnknown,
				};

			public:
				Value(Kind kind) : mKind(kind) {}
				Value(const TArray<CString>& strings) : mKind(kKindStrings), mStrings(strings) {}

			public:
				Kind				mKind;
				TNArray<CString>	mStrings;
		};

	public:
		struct Filter {
			public:
				enum Kind {
					kKindEverything,
					kKindLiteral,
					kKindPathAncestors,
					kKindPrefix,
					kKindRegex,
					kKindRequireAll,
					kKindRequireAny,
					kKindRequireNot,
					kKindSubpath,
					kKindUnknown,
				};

			public:
				Filter(Kind kind, const CString& string = CString::mEmpty) : mKind(kind), mString(string) {}
				Filter(Kind kind, const TArray<Filter>& filters) : mKind(kind), mFilters(filters) {}

			public:
				Kind			mKind;
				CString			mString;
				TNArray<Filter>	mFilters;
		};

	public:
		struct Rule {
			public:
				Rule(bool isAllow, const Filter& filter, bool isConditional) :
					mIsAllow(isAllow), mFilter(filter), mIsConditional(isConditional)
					{}

			public:
				bool	mIsAllow;
				Filter	mFilter;
				bool	mIsConditional;
		};

	public:
		struct Function {
			public:
				Function(const TArray<CString>& parameterNames, const TArray<Node>& body) :
					mParameterNames(parameterNames), mBody(body)
					{}

			public:
				TNArray<CString>	mParameterNames;
				TNArray<Node>		mBody;
		};

	// Methods
	public:
										Internals(const CString& name) : mParameters(composeParameters())
											{ evaluateFile(name + CString(OSSTR(".sb")), false); }

				void					evaluateFile(const CString& fileName, bool isConditional)
											{
												// Read, where an import names a file in the profiles folder
												CFilesystemPath	filesystemPath =
																		fileName.hasPrefix(CString(OSSTR("/"))) ?
																				CFilesystemPath(fileName) :
																				CFilesystemPath(sProfilesFolderPath)
																						.appendingComponent(fileName);
												TVResult<CData>	data = CFileDataSource::readData(CFile(filesystemPath));
												if (!data.hasValue()) {
													// Not readable
													addUnknownRule(isConditional);

													return;
												}

												// Read nodes until the end
												TBuffer<const UInt8>	bytes = data.getValue().getUInt8Buffer();
												UInt64					index = 0;
												TNArray<Node>			nodes;
												while (true) {
													// Check if at end
													skipWhitespaceAndComments(bytes, index);
													if (index == bytes.getCount())
														// Done
														break;

													// Read node
													OV<Node>	node = readNode(bytes, index);
													if (!node.hasValue()) {
														// Not understood
														addUnknownRule(isConditional);

														return;
													}

													// Add
													nodes += *node;
												}

												// Evaluate
												evaluateStatements(nodes, 0, TNDictionary<Value>(), isConditional);
											}
				void					evaluateStatements(const TArray<Node>& nodes, CArray::ItemIndex startIndex,
												const TNDictionary<Value>& bindings, bool isConditional)
											{
												// Iterate nodes
												for (CArray::ItemIndex i = startIndex; i < nodes.getCount(); i++) {
													// Setup
													const	Node&	node = nodes[i];

													// Check statement
													if ((node.mKind != Node::kKindList) || node.mNodes.isEmpty())
														// Not understood
														addUnknownRule(isConditional);
													else if (node.isList(CString(OSSTR("version"))))
														// Has no effect on access
														continue;
													else if (node.isList(CString(OSSTR("allow"))) ||
															node.isList(CString(OSSTR("deny")))) {
														// Collect operations and filters, skipping modifiers such as
														//	(with report)
														bool			appliesToReadingData = false;
														TNArray<Filter>	filters;
														for (CArray::ItemIndex i = 1; i < node.mNodes.getCount(); i++) {
															// Check argument
															const	Node&	argument = node.mNodes[i];
															if (argument.mKind == Node::kKindSymbol)
																// Operation
																appliesToReadingData |=
																		argument.isSymbol(CString(OSSTR("default"))) ||
																				argument.isSymbol(
																						CString(OSSTR("file*"))) ||
																				argument.isSymbol(
																						CString(OSSTR("file-read*"))) ||
																				argument.isSymbol(
																						CString(OSSTR("file-read-data")));
															else if (!argument.isList(CString(OSSTR("with"))))
																// Filter
																filters += composeFilter(argument, bindings);
														}

														// Check if applies to reading data
														if (appliesToReadingData)
															// Add rule, where any of its filters may match
															mRules +=
																	Rule(node.isList(CString(OSSTR("allow"))),
																			filters.isEmpty() ?
																					Filter(Filter::kKindEverything) :
																					Filter(Filter::kKindRequireAny,
																							filters),
																			isConditional);
													} else if (node.isList(CString(OSSTR("import"))) &&
															(node.mNodes.getCount() == 2))
														// Import
														evaluateFile(node.mNodes[1].mString, isConditional);
													else if (node.isList(CString(OSSTR("define"))) &&
															(node.mNodes.getCount() > 2) &&
															(node.mNodes[1].mKind == Node::kKindList) &&
															!node.mNodes[1].mNodes.isEmpty()) {
														// Function
														const	TNArray<Node>&		signature = node.mNodes[1].mNodes;
																TNArray<CString>	parameterNames;
														for (CArray::ItemIndex i = 1; i < signature.getCount(); i++)
															// Add parameter name
															parameterNames += signature[i].mString;

														TNArray<Node>	body;
														for (CArray::ItemIndex i = 2; i < node.mNodes.getCount(); i++)
															// Add body node
															body += node.mNodes[i];

														mFunctions.set(signature[0].mString,
																Function(parameterNames, body));
													} else if ((node.isList(CString(OSSTR("when"))) ||
																	node.isList(CString(OSSTR("unless")))) &&
															(node.mNodes.getCount() > 1)) {
														// Conditional
														Value	condition = evaluateExpression(node.mNodes[1], bindings);
														if (condition.mKind == Value::kKindUnknown)
															// Body may or may not apply
															evaluateStatements(node.mNodes, 2, bindings, true);
														else if ((condition.mKind == Value::kKindStrings) ==
																node.isList(CString(OSSTR("when"))))
															// Body applies
															evaluateStatements(node.mNodes, 2, bindings, isConditional);
													} else if (node.isList(CString(OSSTR("with-filter"))) &&
															(node.mNodes.getCount() > 1)) {
														// Body applies only on Apple-internal Macs, or under a
														//	condition that is not evaluated
														const	Node&	condition = node.mNodes[1];
														if (!condition.isList(CString(OSSTR("system-attribute"))) ||
																(condition.mNodes.getCount() != 2) ||
																!condition.mNodes[1].isSymbol(
																		CString(OSSTR("apple-internal"))))
															// Body may or may not apply
															evaluateStatements(node.mNodes, 2, bindings, true);
													} else if (node.isList(CString(OSSTR("let"))) &&
															(node.mNodes.getCount() > 1) &&
															(node.mNodes[1].mKind == Node::kKindList)) {
														// Bind each name to its value
														TNDictionary<Value>	letBindings(bindings);
														for (TArray<Node>::Iterator iterator =
																		node.mNodes[1].mNodes.getIterator();
																iterator; iterator++) {
															// Check binding
															if ((iterator->mKind == Node::kKindList) &&
																	(iterator->mNodes.getCount() == 2))
																// Bind
																letBindings.set(iterator->mNodes[0].mString,
																		evaluateExpression(iterator->mNodes[1],
																			bindings));
														}

														// Evaluate body
														evaluateStatements(node.mNodes, 2, letBindings, isConditional);
													} else {
														// Check for function
														OR<Function>	function =
																				mFunctions.get(node.mNodes[0].mString);
														if (function.hasReference())
															// Call
															evaluateStatements(function->mBody, 0,
																	composeBindings(*function, node, bindings),
																	isConditional);
														else
															// Not understood
															addUnknownRule(isConditional);
													}
												}
											}
				Value					evaluateExpression(const Node& node, const TNDictionary<Value>& bindings)
											{
												// Check kind
												if (node.mKind == Node::kKindString)
													// String
													return Value(TNArray<CString>(node.mString));
												else if (node.mKind == Node::kKindSymbol) {
													// Bound value
													OR<Value>	value = bindings.get(node.mString);

													return value.hasReference() ? *value : Value(Value::kKindUnknown);
												} else if (node.isList(CString(OSSTR("param"))) &&
														(node.mNodes.getCount() == 2)) {
													// Parameter, false when not set
													OR<CString>	parameter = mParameters.get(node.mNodes[1].mString);

													return parameter.hasReference() ?
															Value(TNArray<CString>(*parameter)) :
															Value(Value::kKindFalse);
												} else if (node.isList(CString(OSSTR("string-append")))) {
													// Concatenate strings
													CString	string;
													for (CArray::ItemIndex i = 1; i < node.mNodes.getCount(); i++) {
														// Evaluate
														Value	value = evaluateExpression(node.mNodes[i], bindings);
														if ((value.mKind != Value::kKindStrings) ||
																(value.mStrings.getCount() != 1))
															// Not a string
															return Value(Value::kKindUnknown);

														// Append
														string += value.mStrings[0];
													}

													return Value(TNArray<CString>(string));
												} else if (node.isList(CString(OSSTR("list")))) {
													// Collect strings
													TNArray<CString>	strings;
													for (CArray::ItemIndex i = 1; i < node.mNodes.getCount(); i++) {
														// Evaluate
														Value	value = evaluateExpression(node.mNodes[i], bindings);
														if (value.mKind != Value::kKindStrings)
															// Not strings
															return Value(Value::kKindUnknown);

														// Add
														strings += value.mStrings;
													}

													return Value(strings);
												} else {
													// Function, whose value is that of its last expression
													OR<Function>	function =
																			!node.mNodes.isEmpty() ?
																					mFunctions.get(
																							node.mNodes[0].mString) :
																					OR<Function>();

													return (function.hasReference() && !function->mBody.isEmpty()) ?
															evaluateExpression(function->mBody.getLast(),
																	composeBindings(*function, node, bindings)) :
															Value(Value::kKindUnknown);
												}
											}
				Filter					composeFilter(const Node& node, const TNDictionary<Value>& bindings)
											{
												// Check form
												if ((node.mKind != Node::kKindList) || node.mNodes.isEmpty())
													// Not understood
													return Filter(Filter::kKindUnknown);

												// Check filter
												OV<Filter::Kind>	pathFilterKind =
																			getPathFilterKind(node.mNodes[0].mString);
												if (pathFilterKind.hasValue())
													// Path filter
													return composePathFilter(*pathFilterKind, node, 1, bindings);
												else if (node.isList(CString(OSSTR("apply")))) {
													// Path filter applied to a list of paths
													OV<Filter::Kind>	appliedPathFilterKind =
																				(node.mNodes.getCount() > 1) ?
																						getPathFilterKind(
																								node.mNodes[1]
																										.mString) :
																						OV<Filter::Kind>();

													return appliedPathFilterKind.hasValue() ?
															composePathFilter(*appliedPathFilterKind, node, 2,
																	bindings) :
															Filter(Filter::kKindUnknown);
												} else if (node.isList(CString(OSSTR("require-all"))) ||
														node.isList(CString(OSSTR("require-any"))) ||
														node.isList(CString(OSSTR("require-not")))) {
													// Combination of filters
													TNArray<Filter>	filters;
													for (CArray::ItemIndex i = 1; i < node.mNodes.getCount(); i++)
														// Add filter
														filters += composeFilter(node.mNodes[i], bindings);

													return Filter(
															node.isList(CString(OSSTR("require-all"))) ?
																	Filter::kKindRequireAll :
																	node.isList(CString(OSSTR("require-any"))) ?
																			Filter::kKindRequireAny :
																			Filter::kKindRequireNot,
															filters);
												} else
													// Not understood
													return Filter(Filter::kKindUnknown);
											}
				Filter					composePathFilter(Filter::Kind kind, const Node& node,
												CArray::ItemIndex startIndex, const TNDictionary<Value>& bindings)
											{
												// Compose a filter for each path, any of which may match
												TNArray<Filter>	filters;
												for (CArray::ItemIndex i = startIndex; i < node.mNodes.getCount();
														i++) {
													// Evaluate
													Value	value = evaluateExpression(node.mNodes[i], bindings);
													if (value.mKind != Value::kKindStrings)
														// Not paths
														return Filter(Filter::kKindUnknown);

													// Add filters
													for (TArray<CString>::Iterator iterator =
																	value.mStrings.getIterator();
															iterator; iterator++)
														// Add filter
														filters += Filter(kind, *iterator);
												}

												return Filter(Filter::kKindRequireAny, filters);
											}
				TNDictionary<Value>		composeBindings(const Function& function, const Node& node,
												const TNDictionary<Value>& bindings)
											{
												// Bind each parameter to its argument, stopping at any parameter
												//	that collects the remaining arguments (". name")
												TNDictionary<Value>	functionBindings;
												for (CArray::ItemIndex i = 0;
														(i < function.mParameterNames.getCount()) &&
																(function.mParameterNames[i] != CString(OSSTR(".")));
														i++)
													// Bind
													functionBindings.set(function.mParameterNames[i],
															((i + 1) < node.mNodes.getCount()) ?
																	evaluateExpression(node.mNodes[i + 1], bindings) :
																	Value(Value::kKindUnknown));

												return functionBindings;
											}
				void					addUnknownRule(bool isConditional)
											{ mRules += Rule(false, Filter(Filter::kKindUnknown), isConditional); }

										// Class methods
		static	TNDictionary<CString>	composeParameters()
											{
												// Compose the parameters the system supplies to a profile for a service
												//	running as this user
												TNDictionary<CString>	parameters;
												parameters.set(CString(OSSTR("HOME")),
														getRealPath(
																CFolder::userHome().getFilesystemPath().getString()));

												char	path[PATH_MAX];
												if (::confstr(_CS_DARWIN_USER_TEMP_DIR, path, sizeof(path)) > 0)
													// Add
													parameters.set(CString(OSSTR("TMPDIR")),
															getRealPath(
																	CString(path, sizeof(path),
																			CString::kEncodingUTF8)));
												if (::confstr(_CS_DARWIN_USER_CACHE_DIR, path, sizeof(path)) > 0)
													// Add
													parameters.set(CString(OSSTR("DARWIN_CACHE_DIR")),
															getRealPath(
																	CString(path, sizeof(path),
																			CString::kEncodingUTF8)));

												return parameters;
											}
		static	OV<Node>				readNode(const TBuffer<const UInt8>& bytes, UInt64& index)
											{
												// Check first character
												if (bytes[index] == '(') {
													// List
													TNArray<Node>	nodes;
													index++;
													while (true) {
														// Check if at end of list
														skipWhitespaceAndComments(bytes, index);
														if (index == bytes.getCount())
															// Unterminated
															return OV<Node>();
														else if (bytes[index] == ')') {
															// End of list
															index++;

															return OV<Node>(Node(nodes));
														}

														// Read node
														OV<Node>	node = readNode(bytes, index);
														if (!node.hasValue())
															// Not understood
															return OV<Node>();

														// Add
														nodes += *node;
													}
												} else if (bytes[index] == ')')
													// Unbalanced
													return OV<Node>();
												else if ((bytes[index] == '"') ||
														((bytes[index] == '#') && ((index + 1) < bytes.getCount()) &&
																(bytes[index + 1] == '"'))) {
													// String, where a regular expression (#"...") keeps its
													//	backslashes
													bool	isRegularExpression = bytes[index] == '#';
													index += isRegularExpression ? 2 : 1;

													UInt64	startIndex = index;
													while ((index < bytes.getCount()) && (bytes[index] != '"'))
														// Next character, skipping any escaped one
														index += (bytes[index] == '\\') ? 2 : 1;
													if (index >= bytes.getCount())
														// Unterminated
														return OV<Node>();

													CString	string(*bytes + startIndex, index - startIndex,
																	CString::kEncodingUTF8);
													index++;

													return OV<Node>(
															Node(Node::kKindString,
																	isRegularExpression ?
																			string :
																			string
																					.replacingSubStrings(
																							CString(OSSTR("\\\"")),
																							CString(OSSTR("\"")))
																					.replacingSubStrings(
																							CString(OSSTR("\\\\")),
																							CString(OSSTR("\\")))));
												} else {
													// Symbol, up to whitespace or a parenthesis
													UInt64	startIndex = index;
													while ((index < bytes.getCount()) && !::isspace(bytes[index]) &&
															(bytes[index] != '(') && (bytes[index] != ')'))
														// Next character
														index++;

													return OV<Node>(
															Node(Node::kKindSymbol,
																	CString(*bytes + startIndex, index - startIndex,
																			CString::kEncodingUTF8)));
												}
											}
		static	void					skipWhitespaceAndComments(const TBuffer<const UInt8>& bytes, UInt64& index)
											{
												// Skip whitespace and comments (; to the end of the line)
												while (index < bytes.getCount()) {
													// Check character
													if (::isspace(bytes[index]))
														// Whitespace
														index++;
													else if (bytes[index] == ';') {
														// Comment
														while ((index < bytes.getCount()) && (bytes[index] != '\n'))
															// Next character
															index++;
													} else
														// Done
														return;
												}
											}
		static	OV<Filter::Kind>		getPathFilterKind(const CString& name)
											{
												// Check name
												if (name == CString(OSSTR("literal")))
													return OV<Filter::Kind>(Filter::kKindLiteral);
												else if (name == CString(OSSTR("path-ancestors")))
													return OV<Filter::Kind>(Filter::kKindPathAncestors);
												else if (name == CString(OSSTR("prefix")))
													return OV<Filter::Kind>(Filter::kKindPrefix);
												else if (name == CString(OSSTR("regex")))
													return OV<Filter::Kind>(Filter::kKindRegex);
												else if (name == CString(OSSTR("subpath")))
													return OV<Filter::Kind>(Filter::kKindSubpath);
												else
													return OV<Filter::Kind>();
											}
		static	OV<bool>				match(const Filter& filter, const CString& path)
											{
												// Check kind.  No value means not known.
												switch (filter.mKind) {
													case Filter::kKindEverything:
														// Everything
														return OV<bool>(true);

													case Filter::kKindLiteral:
														// Exactly this path
														return OV<bool>(path == filter.mString);

													case Filter::kKindPathAncestors:
														// This path or any folder containing it
														return OV<bool>((path == filter.mString) ||
																(path == CString(OSSTR("/"))) ||
																filter.mString.hasPrefix(path + CString(OSSTR("/"))));

													case Filter::kKindPrefix:
														// Paths starting with this string
														return OV<bool>(path.hasPrefix(filter.mString));

													case Filter::kKindRegex: {
														// Paths matching this regular expression
														regex_t	regex;
														if (::regcomp(&regex, *filter.mString.getUTF8String(),
																REG_EXTENDED | REG_NOSUB) != 0)
															// Not understood
															return OV<bool>();

														bool	isMatch =
																		::regexec(&regex, *path.getUTF8String(), 0, nil,
																				0) == 0;
														::regfree(&regex);

														return OV<bool>(isMatch);
													}

													case Filter::kKindRequireAll: {
														// Every filter matches
														OV<bool>	result(true);
														for (TArray<Filter>::Iterator iterator =
																		filter.mFilters.getIterator();
																iterator; iterator++) {
															// Check filter
															OV<bool>	isFilterMatch = match(*iterator, path);
															if (!isFilterMatch.hasValue())
																// May not match
																result.removeValue();
															else if (!*isFilterMatch)
																// No match
																return OV<bool>(false);
														}

														return result;
													}

													case Filter::kKindRequireAny: {
														// Any filter matches
														OV<bool>	result(false);
														for (TArray<Filter>::Iterator iterator =
																		filter.mFilters.getIterator();
																iterator; iterator++) {
															// Check filter
															OV<bool>	isFilterMatch = match(*iterator, path);
															if (!isFilterMatch.hasValue())
																// May match
																result.removeValue();
															else if (*isFilterMatch)
																// Match
																return OV<bool>(true);
														}

														return result;
													}

													case Filter::kKindRequireNot: {
														// The one filter does not match
														OV<bool>	isFilterMatch =
																			(filter.mFilters.getCount() == 1) ?
																					match(filter.mFilters[0], path) :
																					OV<bool>();

														return isFilterMatch.hasValue() ?
																OV<bool>(!*isFilterMatch) : OV<bool>();
													}

													case Filter::kKindSubpath: {
														// This path or anything inside it
														CString	subpath =
																		((filter.mString.getLength() > 1) &&
																						filter.mString.hasSuffix(
																								CString(OSSTR("/")))) ?
																				filter.mString.getSubString(0,
																						filter.mString.getLength()
																								- 1) :
																				filter.mString;

														return OV<bool>((path == subpath) ||
																(subpath == CString(OSSTR("/"))) ||
																path.hasPrefix(subpath + CString(OSSTR("/"))));
													}

													case Filter::kKindUnknown:
														// Not understood
														return OV<bool>();
												}
											}
		static	CString					getRealPath(const CString& path)
											{
												// Resolve symbolic links, as the sandbox matches real paths
												char	realPath[PATH_MAX];

												return (::realpath(*path.getUTF8String(), realPath) != nil) ?
														CString(realPath, sizeof(realPath), CString::kEncodingUTF8) :
														path;
											}

	// Properties
	public:
		TNDictionary<CString>	mParameters;
		TNDictionary<Function>	mFunctions;
		TNArray<Rule>			mRules;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSandboxProfile

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSandboxProfile::CSandboxProfile(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(name);
}

//----------------------------------------------------------------------------------------------------------------------
CSandboxProfile::~CSandboxProfile()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<bool> CSandboxProfile::canAccess(const CFilesystemPath& filesystemPath) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check rules, last first, as a later rule takes precedence over an earlier one
	CString	path = Internals::getRealPath(filesystemPath.getString());
	for (CArray::ItemIndex i = mInternals->mRules.getCount(); i > 0; i--) {
		// Check rule
		const	Internals::Rule&	rule = mInternals->mRules[i - 1];
				OV<bool>			isMatch = Internals::match(rule.mFilter, path);
		if (!isMatch.hasValue() || *isMatch)
			// This rule decides, unless it applies only under a condition that is not evaluated
			return (isMatch.hasValue() && !rule.mIsConditional) ? OV<bool>(rule.mIsAllow) : OV<bool>();
	}

	// No rule applies
	return OV<bool>();
}
