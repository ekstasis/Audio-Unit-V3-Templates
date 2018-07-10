//
//  DSPKernel.hpp
//  AudioUnitV3Template
//
//  Created by mhamilt7 on 10/07/2018.
//  Copyright © 2018 mhamilt7. All rights reserved.
//
//==============================================================================
#ifndef DSPKernel_h
#define DSPKernel_h
//==============================================================================
#import <vector>
//==============================================================================
/*
 DSPKernel Performs our filter signal processing.
 As a non-ObjC class, this is safe to use from render thread.
 */
class DSPKernel
{
public:
    //==========================================================================
    // MARK: Member Functions
    DSPKernel() {}
    
    //==========================================================================
    void init(int channelCount, double inSampleRate)
    {
        channels = channelCount;
        sampleRate = float(inSampleRate);
    }
    //==============================================================================
    /**
     The central DSP Algorithm: this is essentially 'where the magic happens'
     
     @param frameCount from the DSPKernel super class. This is the framesRemaining
     which is the 'AVAudioFrameCount frameCount' directly from
     internalRenderBlock in the main FilterDemo.mm file.
     
     @param bufferOffset  bufferOffset = frameCount - framesRemaining. frameCount
     from internalRenderBlock is copied and subtracted from
     processWithEvents in the DSPKernel in which the call to
     this function is made
     */
    void process(AUAudioFrameCount frameCount, AUAudioFrameCount bufferOffset)
    {
        int channelCount = channels;
        for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) // For each sample.
        {
            int frameOffset = int(frameIndex + bufferOffset);
            for (int channel = 0; channel < channelCount; ++channel)
            {
                float* in  = (float*)inBufferListPtr->mBuffers[channel].mData  + frameOffset;
                float* out = (float*)outBufferListPtr->mBuffers[channel].mData + frameOffset;
                *out = *in; // MARK: Sample Processing
            }
        }
    }
    //==============================================================================
    void processWithEvents(AudioTimeStamp const *timestamp,
                           AUAudioFrameCount frameCount,
                           AURenderEvent const *events,
                           AudioBufferList* inBufferList,
                           AudioBufferList* outBufferList)
    {
        //----------------------------------------------------------------------
        inBufferListPtr = inBufferList;
        outBufferListPtr = outBufferList;
        //----------------------------------------------------------------------
        AUEventSampleTime now = AUEventSampleTime(timestamp->mSampleTime);
        AUAudioFrameCount framesRemaining = frameCount;
        AURenderEvent const *event = events;
        //----------------------------------------------------------------------
        while (framesRemaining > 0)
        {
            // If there are no more events, we can process the entire remaining segment and exit.
            if (event == nullptr)
            {
                AUAudioFrameCount const bufferOffset = frameCount - framesRemaining;
                process(framesRemaining, bufferOffset);
                return;
            }
            
            // **** start late events late.
            auto timeZero = AUEventSampleTime(0);
            auto headEventTime = event->head.eventSampleTime;
            AUAudioFrameCount const framesThisSegment = AUAudioFrameCount(std::max(timeZero, headEventTime - now));
            
            // Compute everything before the next event.
            if (framesThisSegment > 0)
            {
                AUAudioFrameCount const bufferOffset = frameCount - framesRemaining;
                process(framesThisSegment, bufferOffset);
                framesRemaining -= framesThisSegment;    // Advance frames.
                now += AUEventSampleTime(framesThisSegment); // Advance time.
            }
            //            performAllSimultaneousEvents(now, event);
        }
    }
    //==========================================================================
private:
    //==========================================================================
    // MARK: Member Variables
    float sampleRate = 44100.0;
    int channels;
    AudioBufferList* inBufferListPtr = nullptr;
    AudioBufferList* outBufferListPtr = nullptr;
};

#endif /* DSPKernel_h */
