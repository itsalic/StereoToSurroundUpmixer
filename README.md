# Adaptive Stereo To 5.1 Surround Upmixer - VST Plugin
If you have a 5.1 home theatre setup plugged into a Windows PC which doesnt play stereo audio through some of the speakers, look no further as this is the solution for you!


------------

## The Problem
Most audio recievers have built in capability to upmix a 2 channel stereo signal to fill all its channels (there is also lots of existing software to do this also). However if the audio signal is already multi-channel, it won't run it through any sort of upmixing algorithm. It is often experienced that when Windows is set to output mutli-channel (5.1, 7.1 etc.), it will output multi-channel **always**; even when only stereo audio is playing (unless the audio source takes exclusive control of window's audio output). And windows makes no attempt to fill in the empty channels. Meaning only Front left and right channels will play the audio.
## The Solution
I designed a solution, which will seamlessly play true surround, or alternatively fill a multi-channel surround output depending on the active audio. This means there is no need for switching modes or clicking any buttons, if you want to change modes.
### Lets get technical...
This VST plugin take's window's multi-channel audio output, and checks on a channel per channel basis to see if it is empty. If a true 5.1 audio signal is present, the output will be unchanged and all of this plugin's processings will be bypassed. However, if the a channel is empty, it will get fill the channel with a corresponding channel.
As below:

         L                                C                                R                      LFE (Sub)
    (unchanged)                 (50% Left + 50% Right)                (unchanged)           (50% Left + 50% Right)

        RL                                                                RR
     (L Copy)                                                          (R Copy)
                    
## The Controls
As most upmixers do, it provides some controls to filter the output frequencies sent to the speakers, surround speakers generally will output the mid & high frequencies, and the sub is responsible for reproducing the low frequencies. The crossover frequency control defines the frequency (Hz) in which the bass sounds are rolled off the surround speakers and the trebels are rolled off the subwoofer channel. Rolling the dial to the left will introduce more bass into the surround speakers, but subtract some of the bass/mids from the Sub. The crossover frequency will vary between home theatre setups, but if you aren't sure then leave it as it is. If you want the bass sound coming from the surround speakers also, check the 'Full-range surround speakers' button, this will bypass the bass cut-off on the surround channels.

![Screenshot 2022-03-06 155201 2235](https://user-images.githubusercontent.com/15713334/156935549-2b07be7a-1b05-45e7-a194-6d0095fe9d96.png)

# Installation Guide
If you are unfamiliar with what a VST Plugin is or how to use it, don't worry its super simple!
First download and install EqualizerAPO ([Download and guide here](https://equalizerapo.com/)) and enable it on the desired audio device that you want this plugin to work with.

Next download the packaged VST plugin below:
[Download](https://github.com/itsalic/StereoToSurroundUpmixer/raw/Distributables/StereoToSurround.zip)
Extract the plugin .dll and place it in your Equaliser APO plugin folder (Default location: C:\Program Files\EqualizerAPO\VSTPlugins)
Open the EqualizerAPO configurator, and add a VST Plugin

![image](https://user-images.githubusercontent.com/15713334/156935531-4aa564fe-ae58-40a9-b1a8-936b9bcbc437.png)

Select the StereoToSurround.dll and that's it! You're done and it should all be working!

This is my first time creating a VST Plugin (built with the JUCE framework), and i'm rather inexperienced in C++ (I'm a java developer mainly).
If you have any improvement ideas or code suggestions please create an Issue thread and I will make sure to read them.

### Known Bugs
Currently, the plugin does not know the output channels Windows is set to (if you know how to do that, please let me know how). If Windows is set to output Stereo, and the plugin is left on, Window's will only take the L & R channels output from this plugin, so there will not be any bass present (as it will have been filtered off the normal channels. If you wish to keep this plugin active while using Windows in Stereo mode, click the 'Full-range Surround Speakers' button, and this will preserve all of the bass, allowing the audio reciever to upmix the full stereo sound.
