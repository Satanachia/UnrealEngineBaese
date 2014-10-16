// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "AvfMediaPrivatePCH.h"

#if PLATFORM_MAC
#include "CocoaThread.h"
#endif

/* FAvfMediaPlayer structors
 *****************************************************************************/

FAvfMediaPlayer::FAvfMediaPlayer()
{
    CurrentTime = FTimespan::Zero();
    Duration = FTimespan::Zero();
	MediaUrl = FString();
    
    PlayerItem = nil;
    MediaPlayer = nil;
}


/* IMediaInfo interface
 *****************************************************************************/

FTimespan FAvfMediaPlayer::GetDuration() const
{
	return Duration;
}


TRange<float> FAvfMediaPlayer::GetSupportedRates( EMediaPlaybackDirections Direction, bool Unthinned ) const
{
	float MaxRate = 1.0f;
	float MinRate = 0.0f;
    
	return TRange<float>(MinRate, MaxRate);
}


FString FAvfMediaPlayer::GetUrl() const
{
	return MediaUrl;
}


bool FAvfMediaPlayer::SupportsRate( float Rate, bool Unthinned ) const
{
    return GetSupportedRates(EMediaPlaybackDirections::Forward, true).Contains( Rate );
}


bool FAvfMediaPlayer::SupportsScrubbing() const
{
	return false;
}


bool FAvfMediaPlayer::SupportsSeeking() const
{
	return false;
}


/* IMediaPlayer interface
 *****************************************************************************/

void FAvfMediaPlayer::Close()
{
    CurrentTime = 0;
	MediaUrl = FString();
    
    if( PlayerItem != nil )
    {
        PlayerItem = nil;
    }

    if( MediaPlayer != nil )
    {
        MediaPlayer = nil;
    }
    
	Tracks.Reset();
    
    Duration = CurrentTime = FTimespan::Zero();
    
#if PLATFORM_MAC
    GameThreadCall(^{
#elif PLATFORM_IOS
    // Report back to the game thread whether this succeeded.
    [FIOSAsyncTask CreateTaskWithBlock : ^ bool(void){
#endif
        // Displatch on the gamethread that we have closed the video.
        ClosedEvent.Broadcast();
         
#if PLATFORM_IOS
        return true;
    }];
#elif PLATFORM_MAC
    });
#endif

}


const IMediaInfo& FAvfMediaPlayer::GetMediaInfo() const
{
	return *this;
}


float FAvfMediaPlayer::GetRate() const
{
    return CurrentRate;
}


FTimespan FAvfMediaPlayer::GetTime() const
{
    return CurrentTime;
}


const TArray<IMediaTrackRef>& FAvfMediaPlayer::GetTracks() const
{
	return Tracks;
}


bool FAvfMediaPlayer::IsLooping() const 
{
    return false;
}


bool FAvfMediaPlayer::IsPaused() const
{
    return FMath::IsNearlyZero( [MediaPlayer rate] );
}


bool FAvfMediaPlayer::IsPlaying() const
{
    return (MediaPlayer != nil) && (1.0f == MediaPlayer.rate) && (Tracks.Num() > 0) && (CurrentTime <= Duration);
}


bool FAvfMediaPlayer::IsReady() const
{
    // To be ready, we need the AVPlayer setup
    bool bIsReady = (MediaPlayer != nil) && ([MediaPlayer status] == AVPlayerStatusReadyToPlay);
    // A player item setup,
    bIsReady &= (PlayerItem != nil) && ([PlayerItem status] == AVPlayerItemStatusReadyToPlay);
    
    // and all tracks to be setup and ready
    for( const IMediaTrackRef& Track : Tracks )
    {
        FAvfMediaVideoTrack* AVFTrack = (FAvfMediaVideoTrack*)&Track.Get();
        if( AVFTrack != nil )
        {
            bIsReady &= AVFTrack->IsReady();
        }
    }
    
    return bIsReady;
}


/* FAvfMediaPlayer implementation
 *****************************************************************************/


bool FAvfMediaPlayer::Open( const FString& Url )
{
    // Cache off the url for use on other threads.
    FString CachedUrl = Url;
    
    Close();
    
    bool bSuccessfullyOpenedMovie = false;

    FString Ext = FPaths::GetExtension(Url);
    FString BFNStr = FPaths::GetBaseFilename(Url);
    
#if PLATFORM_MAC
    NSURL* nsMediaUrl = [NSURL fileURLWithPath:Url.GetNSString()];
#elif PLATFORM_IOS
    NSURL* nsMediaUrl = [[NSBundle mainBundle] URLForResource:BFNStr.GetNSString() withExtension:Ext.GetNSString()];
#endif
    
    if( nsMediaUrl != nil )
    {
        MediaUrl = FPaths::GetCleanFilename(Url);
        
        MediaPlayer = [[AVPlayer alloc] init];
        
        if( MediaPlayer )
        {
            PlayerItem = [AVPlayerItem playerItemWithURL: nsMediaUrl];
            if( PlayerItem != nil )
            {
                Duration = FTimespan::FromSeconds( CMTimeGetSeconds( PlayerItem.asset.duration ) );

                [[PlayerItem asset] loadValuesAsynchronouslyForKeys: @[@"tracks"] completionHandler:^
                {
                    NSError* Error = nil;
                    if( [[PlayerItem asset] statusOfValueForKey:@"tracks" error:&Error] == AVKeyValueStatusLoaded )
                    {
                        NSArray* VideoTracks = [[PlayerItem asset] tracksWithMediaType:AVMediaTypeVideo];
                        if( VideoTracks.count > 0 )
                        {
                            AVAssetTrack* VideoTrack = [VideoTracks objectAtIndex: 0];
                            Tracks.Add( MakeShareable( new FAvfMediaVideoTrack(VideoTrack) ) );
     
#if PLATFORM_MAC
                            GameThreadCall(^{
#elif PLATFORM_IOS
                            // Report back to the game thread whether this succeeded.
                            [FIOSAsyncTask CreateTaskWithBlock : ^ bool(void){
#endif
                                // Displatch on the gamethread that we have opened the video.
                                OpenedEvent.Broadcast( CachedUrl );
                         
#if PLATFORM_IOS
                                return true;
                            }];
#elif PLATFORM_MAC
                            });
#endif
                        }
                    }
                    else
                    {
                        NSDictionary *userInfo = [Error userInfo];
                        NSString *errstr = [[userInfo objectForKey : NSUnderlyingErrorKey] localizedDescription];
                        UE_LOG(LogAvfMedia, Display, TEXT("Failed to load video tracks. [%s]"), *FString(errstr));
                    }
                }];
            
                [MediaPlayer replaceCurrentItemWithPlayerItem: PlayerItem];
            
                [[MediaPlayer currentItem] seekToTime:kCMTimeZero];
                MediaPlayer.rate = 0.0;
            
                bSuccessfullyOpenedMovie = true;
            }
            else
            {
                UE_LOG(LogAvfMedia, Warning, TEXT("Failed to open player item with Url:"), *Url);
            }
        }
        else
        {
            UE_LOG(LogAvfMedia, Error, TEXT("Failed to create instance of an AVPlayer"));
        }
    }
    else
    {
        UE_LOG(LogAvfMedia, Warning, TEXT("Failed to open Media file:"), *Url);
    }
    
    
    return bSuccessfullyOpenedMovie;
}


bool FAvfMediaPlayer::Open( const TSharedRef<TArray<uint8>>& Buffer, const FString& OriginalUrl )
{
    return false;
}


bool FAvfMediaPlayer::Seek( const FTimespan& Time )
{
    // We need to find a suitable way to seek using avplayer and AVAssetReader,
/*
    CurrentTime = Time;

    [[MediaPlayer currentItem] seekToTime:CMTimeMakeWithSeconds(CurrentTime.GetSeconds(), 1000)];

    for( IMediaTrackRef& Track : Tracks )
    {
        FAvfMediaVideoTrack* AVFTrack = (FAvfMediaVideoTrack*)&Track.Get();
        if( AVFTrack != nil )
        {
            AVFTrack->SeekToTime( [[MediaPlayer currentItem] currentTime] );
        }
    }
*/
    return false;
}


bool FAvfMediaPlayer::SetLooping( bool Looping )
{
	return false;
}


bool FAvfMediaPlayer::SetRate( float Rate )
{
    CurrentRate = Rate;
    [MediaPlayer setRate: CurrentRate];
    return true;
}


/* FTickerBase interface
 *****************************************************************************/

bool FAvfMediaPlayer::ShouldAdvanceFrames() const
{
    return IsReady() && IsPlaying();
}

bool FAvfMediaPlayer::Tick( float DeltaTime )
{
    if( ShouldAdvanceFrames() )
    {
        CurrentTime = FTimespan::FromSeconds(CMTimeGetSeconds([[MediaPlayer currentItem] currentTime]));
        
        for (IMediaTrackRef& Track : Tracks)
        {
            FAvfMediaVideoTrack* AVFTrack = (FAvfMediaVideoTrack*)&Track.Get();
        
            if (AVFTrack != nil)
            {
                AVFTrack->ReadFrameAtTime([[MediaPlayer currentItem] currentTime]);
            }
        }
    }

    
    return true;
}
