//----------------------------------------------------------------------------------------------------------------------
//	TTreeBuilder.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TTreeBuilder

template <typename G, typename GInfo, typename T, typename TInfo> class TTreeBuilder {
	// Procs
	public:
		typedef	I<G>	(*CreateGroupProc)(const CString& subPath, const OV<GInfo>& info,
								const OV<TArray<I<G> > >& groups, const OV<TArray<I<T> > >& items, void* userData);
		typedef	I<T>	(*CreateItemProc)(const CString& subPath, const OV<TInfo>& info, void* userData);

	// GroupTracker
	private:
		class GroupTracker {
			// Methods
			public:
						// Lifecycle methods
						GroupTracker(const CString& subPath, const OV<GInfo>& info = OV<GInfo>()) :
							mSubPath(subPath), mInfo(info)
							{}

						// Instance methods
				void	setInfo(const OV<GInfo>& info)
							{ mInfo = info; }

				void	add(const I<GroupTracker>& groupTracker)
							{
								// Setup
								if (!mChildGroupTrackers.hasValue())
									// First child group tracker
									mChildGroupTrackers.setValue(TNArray<I<GroupTracker> >());

								// Add
								*mChildGroupTrackers += groupTracker;
							}
				void	add(const CString& subPath, const OV<TInfo>& info = OV<TInfo>())
							{
								// Setup
								if (!mChildItemInfos.hasValue())
									// First child item info
									mChildItemInfos.setValue(TNDictionary<OV<TInfo> >());

								// Add
								mChildItemInfos->set(*getLastPathComponent(subPath), info);
							}

				I<G>	createGroup(CreateGroupProc createGroupProc, CreateItemProc createItemProc, void* userData)
								const
							{
								// Create child groups
								OV<TArray<I<G> > >	childGroups;
								if (mChildGroupTrackers.hasValue()) {
									// Iterate child group trackers
									TNArray<I<G> >	groups;
									for (typename TArray<I<GroupTracker> >::Iterator iterator =
													mChildGroupTrackers->getIterator();
											iterator; iterator++)
										// Create group
										groups += (*iterator)->createGroup(createGroupProc, createItemProc, userData);

									childGroups.setValue(groups);
								}

								// Create child items
								OV<TArray<I<T> > >	childItems;
								if (mChildItemInfos.hasValue()) {
									// Setup
									AssertFailIf(createItemProc == nil);

									// Iterate child item infos
									TNArray<I<T> >	items;
									for (typename TDictionary<OV<TInfo> >::Iterator iterator =
													mChildItemInfos->getIterator();
											iterator; iterator++)
										// Create item
										items +=
												createItemProc(getAppendingPathComponent(mSubPath, iterator.getKey()),
														iterator.getValue(), userData);

									childItems.setValue(items);
								}

								return createGroupProc(mSubPath, mInfo, childGroups, childItems, userData);
							}

			// Properties
			private:
				CString							mSubPath;
				OV<GInfo>						mInfo;

				OV<TNArray<I<GroupTracker> > >	mChildGroupTrackers;
				OV<TNDictionary<OV<TInfo> > >	mChildItemInfos;
		};

	// Methods
	public:
							// Lifecycle methods
							TTreeBuilder(CreateGroupProc createGroupProc, CreateItemProc createItemProc = nil,
									void* userData = nil) :
								mCreateGroupProc(createGroupProc), mCreateItemProc(createItemProc),
										mUserData(userData)
								{
									// Create root group tracker
									mGroupTrackerMap.set(CString::mEmpty,
											I<GroupTracker>(new GroupTracker(CString::mEmpty)));
								}

							// Instance methods
				I<G>		getRootGroup() const
								{ return (*mGroupTrackerMap[CString::mEmpty])->createGroup(mCreateGroupProc,
										mCreateItemProc, mUserData); }

				void		addGroup(const CString& subPath, const OV<GInfo>& info = OV<GInfo>())
								{
									// Check if already have a GroupTracker
									const	OR<I<GroupTracker> >	existingGroupTracker = mGroupTrackerMap[subPath];
									if (existingGroupTracker.hasReference())
										// Have GroupTracker
										(*existingGroupTracker)->setInfo(info);
									else {
										// Setup
										I<GroupTracker>	groupTracker(new GroupTracker(subPath, info));
										CString			group = getDeletingLastPathComponent(subPath);

										// Add group tracker
										mGroupTrackerMap.set(subPath, groupTracker);

										// Check if have GroupTracker for group
										if (!mGroupTrackerMap.contains(group))
											// Add Group for parent
											addGroup(group);

										// Add to parent
										(*mGroupTrackerMap[group])->add(groupTracker);
									}
								}
				void		addItem(const CString& path, const OV<TInfo>& info = OV<TInfo>())
								{
									// Setup
									CString	group = getDeletingLastPathComponent(path);

									// Check if have GroupTracker for group
									if (!mGroupTrackerMap.contains(group))
										// Add Group
										addGroup(group);

									// Add item to parent group tracker
									(*mGroupTrackerMap[group])->add(path, info);
								}

	private:
							// Class methods
		static	OV<CString>	getLastPathComponent(const CString& path)
								{ return !path.isEmpty() ?
										OV<CString>(path.components(CString::mSlash).getLast()) : OV<CString>(); }
		static	CString		getDeletingLastPathComponent(const CString& path)
								{
									// Setup
									TArray<CString>	pathComponents = path.components(CString::mSlash);
									CString::Length	pathLength = path.getLength();
									CString::Length	lastPathComponentLength = pathComponents.getLast().getLength();

									return (pathComponents.getCount() > 1) ?
											path.getSubString(0, pathLength - lastPathComponentLength - 1) :
											CString::mEmpty;
								}
		static	CString		getAppendingPathComponent(const CString& path, const CString& pathComponent)
								{
									// Return correct path considering:
									//	- path may be empty
									//	- path may end in "/"
									//	- pathComponent may be empty
									//	- pathComponent may start with "/"
									if (path.isEmpty())
										// Path is empty
										return pathComponent;
									else if (pathComponent.isEmpty())
										// Path component is empty
										return path;
									else
										// Neither are empty
										return (path.hasSuffix(CString::mSlash) ? path : path + CString::mSlash) +
												(pathComponent.hasPrefix(CString::mSlash) ?
														pathComponent.getSubString(1) : pathComponent);
								}

	// Properties
	private:
		CreateGroupProc					mCreateGroupProc;
		CreateItemProc					mCreateItemProc;
		void*							mUserData;

		TNDictionary<I<GroupTracker> >	mGroupTrackerMap;
};
