//----------------------------------------------------------------------------------------------------------------------
//	CAudioPlayerReaderThread.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioPlayer.h"
#include "CQueue.h"
#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioPlayerReaderThread

//class CAudioPlayer;
class CAudioPlayerReaderThreadInternals;
class CAudioPlayerReaderThread : public CThread {
	// Procs
	public:
		typedef	void	(*ErrorProc)(const SError& error, void* userData);

	// Methods
	public:
				// MARK: Lifecycle methods
				CAudioPlayerReaderThread(CAudioPlayer& audioPlayer, CSRSWBIPSegmentedQueue& queue, UInt32 bytesPerFrame,
						UInt32 maxOutputFrames, ErrorProc errorProc, void* procsUserData);
				~CAudioPlayerReaderThread();

				// MARK: CThread methods
		void	run();

				// MARK: Instance methods
		void	seek(UniversalTimeInterval timeInterval, UInt32 maxFrames);
		void	resume();
		void	noteQueueReadComplete();
		bool	getDidReachEndOfData() const;
		void	stopReading();
		void	shutdown();

	// Properties
	private:
		CAudioPlayerReaderThreadInternals*	mInternals;
};
