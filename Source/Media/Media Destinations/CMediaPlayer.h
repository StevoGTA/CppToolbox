//----------------------------------------------------------------------------------------------------------------------
//	CMediaPlayer.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioPlayer.h"
#include "CMediaDestination.h"
#include "CVideoFrameStore.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPlayer

/*
	CMediaPlayer plays one media item (audio, and video if present).  It is driven with the methods in this class
	and observed through the Info callbacks.  It coordinates playback and preview and owns the reporting boundary:
	the only things reported are real playback events - a preview (a brief sounding of a position during an
	interaction) is handled internally and is never reported as playback.

	CONTROL
		Transport
			play()							start, or resume after pause(), real playback
			pause()							suspend playback, keeping the current position
			stop()							halt playback and return the playhead to the segment start (not playing)
			restart()						while playing, seamlessly replay from the segment start (for looping)
			isPlaying()						true while real playback is active

		Position
			seek(t)							move the playhead to t (see INTERACTIONS for behavior while interacting)
			getCurrentTimeInterval()

		Media segment
			getMediaSegment()				the current segment, or none
			setMediaSegment(mediaSegment)	the [start, duration] window playback is restricted to
			setMediaSegment()				whole source will be played

		Looping
			setLoopCount(n)					replay the segment n times before finishing (unset => play through once)

	EVENTS (via Info struct)
		finished()							playback reached the end of the segment, after exhausting any loops

		audioPositionUpdated(t)				playback - or an interaction's target - reached t
		audioError(e)

		videoFrameUpdated(frame)			playback - or an interaction's target - reached the frame
		videoError(e)

	INTERACTIONS
		Two interactions handled at this layer provide an audio preview during the interaction.  Each is a
		start -> change(s) -> finish bracket that remembers whether playback was active when it began and restores
		that on finish.

			Interactively changing the media segment:
				mediaSegmentWillChange() ... setMediaSegment(seg) [0..n] ... mediaSegmentDidChange()
			Interactively moving the playhead:
				startSeek() ... seek(t) [0..n] ... finishSeek()

		While bracketed, each change may briefly preview ~mPreviewDuration of audio at the new position; starting a
		preview aborts any in-flight preview immediately, so the preview always reflects the most recent call rather
		than falling behind it.  Only the target position of each change is reported through audioPositionUpdated, so
		the playhead follows each change - the audible playback through a preview, and its end, are not reported (no
		streaming positions, no finished()).

		Outside a bracket, setMediaSegment(seg) and seek(t) take effect immediately: seek repositions and
		setMediaSegment re-scopes to the segment, each auditioning the target (~mPreviewDuration) when NOT playing, or
		repositioning and continuing when playing.

	EXPECTED FLOWS   (call sequence => events)

		Basic playback: (Typically a user click or keyboard command)
			play() =>
				audioPositionUpdated(...) as it advances
				finished() at the segment end (or, with a loop count set, it silently restarts and keeps reporting
					positions until the loops are exhausted).
			pause() =>
				no event.   play() again will resume from the current position.

		Seek while NOT playing: (User clicks in UI to change playback position, audible preview)
			startSeek();
			seek(t)...;
			finishSeek() =>
				preview and audioPositionUpdated(t) for each t.  finishSeek() leaves the playhead parked at the last t.

		Seek while playing: (User clicks in UI to change playback position, no preview - just continue playing)
			seek(t)... => immediately repositions to each t (clamped to the segment)
				audioPositionUpdated(t) for each t.

		Seek while playing: (User is interacting with a playback position control, playback stops at interaction start,
				resumes at interaction end, audible preview)
			startSeek() => audible output suspended (no event);
			seek(t)...;
			finishSeek() =>
				preview and audioPositionUpdated(t) for each t.  finishSeek() resumes playback from the last t.

		Set the media segment while NOT playing: (User performs an action to set the media segment in one go, audible
				preview)
			setMediaSegment(seg) =>
				audioPositionUpdated(start).
				setMediaSegment(seg) leaves the playhead parked at the seg start (having auditioned it).

		Change the media segment while NOT playing: (User is interacting with a playback selection control, audible
				preview)
			mediaSegmentWillChange();
			setMediaSegment(seg)...;
			mediaSegmentDidChange() =>
				preview and audioPositionUpdated(start) when seg start is different from the current t.
				mediaSegmentDidChange() leaves the playhead parked at the last seg start.

		Change the media segment while playing: (User is interacting with a playback selection control, playback stops
				at interaction start, resumes at interaction end, audible preview)
			mediaSegmentWillChange() => audible output suspended (no event);
			setMediaSegment(seg)...;
			mediaSegmentDidChange() =>
				preview and audioPositionUpdated(start) when seg start is different from the current t.
				mediaSegmentDidChange() resumes playback from the last seg start.
*/

class CMediaPlayer : public TMediaDestination<CAudioPlayer, CVideoFrameStore> {
	// Info
	public:
		struct Info {
			// Procs
			typedef	void	(*AudioPositionUpdatedProc)(UniversalTimeInterval position, void* userData);
			typedef	void	(*AudioErrorProc)(const SError& error, void* userData);

			typedef	void	(*VideoFrameUpdatedProc)(const CVideoFrame& videoFrame, void* userData);
			typedef	void	(*VideoErrorProc)(const SError& error, void* userData);

			typedef	void	(*FinishedProc)(void* userData);

					// Lifecycle methods
					Info() :
						mAudioPositionUpdatedProc(nil), mAudioErrorProc(nil), mVideoFrameUpdatedProc(nil),
								mVideoErrorProc(nil), mFinishedProc(nil), mUserData(nil)
						{}
					Info(AudioPositionUpdatedProc audioPositionUpdatedProc, AudioErrorProc audioErrorProc,
							VideoFrameUpdatedProc videoFrameUpdatedProc, VideoErrorProc videoErrorProc,
							FinishedProc finishedProc, void* userData) :
						mAudioPositionUpdatedProc(audioPositionUpdatedProc), mAudioErrorProc(audioErrorProc),
								mVideoFrameUpdatedProc(videoFrameUpdatedProc), mVideoErrorProc(videoErrorProc),
								mFinishedProc(finishedProc), mUserData(userData)
						{}
					Info(const Info& other) :
						mAudioPositionUpdatedProc(other.mAudioPositionUpdatedProc),
								mAudioErrorProc(other.mAudioErrorProc),
								mVideoFrameUpdatedProc(other.mVideoFrameUpdatedProc),
								mVideoErrorProc(other.mVideoErrorProc),
								mFinishedProc(other.mFinishedProc),
								mUserData(other.mUserData)
						{}

					// Instance methods
			void	audioPositionUpdated(UniversalTimeInterval position) const
						{
							// Check for proc
							if (mAudioPositionUpdatedProc != nil)
								// Call proc
								mAudioPositionUpdatedProc(position, mUserData);
						}
			void	audioError(const SError& error) const
						{
							// Check for proc
							if (mAudioErrorProc != nil)
								// Call proc
								mAudioErrorProc(error, mUserData);
						}

			void	videoFrameUpdated(const CVideoFrame& videoFrame) const
						{
							// Check for proc
							if (mVideoFrameUpdatedProc != nil)
								// Call proc
								mVideoFrameUpdatedProc(videoFrame, mUserData);
						}
			void	videoError(const SError& error) const
						{
							// Check for proc
							if (mVideoErrorProc != nil)
								// Call proc
								mVideoErrorProc(error, mUserData);
						}

			void	finished() const
						{
							// Check for proc
							if (mFinishedProc != nil)
								// Call proc
								mFinishedProc(mUserData);
						}

			// Properties
			private:
				AudioPositionUpdatedProc	mAudioPositionUpdatedProc;
				AudioErrorProc				mAudioErrorProc;

				VideoFrameUpdatedProc		mVideoFrameUpdatedProc;
				VideoErrorProc				mVideoErrorProc;

				FinishedProc				mFinishedProc;

				void*						mUserData;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
												// Lifecycle methods
												CMediaPlayer(CSRSWMessageQueues& messageQueues, const Info& info);
												~CMediaPlayer();

												// CMediaDestination methods
						void					add(const I<CAudioDestination>& audioDestination, UInt32 trackIndex);

						void					add(const I<CVideoDestination>& videoDestination, UInt32 trackIndex);

						void					setMediaSegment(
														const OV<SMedia::Segment>& mediaSegment =
																OV<SMedia::Segment>());

						void					seek(UniversalTimeInterval timeInterval);

												// Instance methods
		virtual			I<CAudioPlayer>			newAudioPlayer(const CString& identifier, UInt32 trackIndex);
		virtual			void					setAudioGain(Float32 audioGain);

		virtual			I<CVideoFrameStore>		newVideoFrameStore(const CString& identifier, UInt32 trackIndex);

		virtual			void					setLoopCount(OV<UInt32> loopCount = OV<UInt32>());

						UniversalTimeInterval	getCurrentTimeInterval() const;
				const	OV<UInt32>&				getCurrentFrameIndex() const;

		virtual			void					play();
		virtual			void					pause();
		virtual			bool					isPlaying() const;

						void					startSeek();
						void					finishSeek();

						void					mediaSegmentWillChange();
						void					mediaSegmentDidChange();

						void					stop();
						void					restart();

	// Properties
	private:
		Internals*	mInternals;
};
