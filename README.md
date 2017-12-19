# BaconPlugs

BaconPlugs are my set (currently of size 4) of EuroRack style plugins for 
[VCVRack](http://www.vcvrack.com). The modules are mostly just me noodling around.
All the source is here, releases under an Apache 2.0 license. You are free to use
these modules as you see fit. If you happen to use them to make music, please
do let me know, either here or by tagging me on twitter (@baconpaul).

I build regularly on MacOS and check that it builds on Linux from time to time.

## HarMoNee

HarMoNee is a plugin which takes a 1v/oct CV signal and outputs two signals,
one which is the original, and the second which is modified by a musical amount,
like a minor 3rd. It spans plus or minus one octave, and is controlled by toggles.

The toggles are additive. So if you want a fourth, choose a major third and a half step 
both. You get the idea. Here's the sample patch I use for testing in the current version.

![Example HarMoNee patch](doc/HarMoNee.png)

## Glissinator

Glissinator takes a control voltage which is undergoing change and smooths out that
change with a linear glissando. It is not triggered by a gate, just by differences
in the input CV. It never jumps discontinously, so if the CV changes "target" value
mid-gliss, the whole thing turns around. The slider will give you between 0 and 1 seconds
of gliss time. Here's a sample patch.

![Example Glissinator Patch](doc/Glissinator.png)

## ALingADing 

ALingADing is a simulation of a Ring Modulator based on [this paper by Julian Parker](http://recherche.ircam.fr/pub/dafx11/Papers/66_e.pdf).
Rather tha following Parker's use of a few polynomials to simulate his diode, 
I basically use an implementaion of a softmax, eyeballing the parameters to roughly meet the figure in his
book. The only control is a wet/dry mix (where wet is the signal modulated by carrier
and dry is just the signal). Sloppy, sure, but it sounds kinda cool. Here's a sample patch.

![Example ALingADing Patch](doc/ALingADing.png)

## Bitulator

Bitulator is really just me screwing around with some math on the input. It has two
functions. Firstly, it "quantizes" to a smaller number of "bits", but does it in a
wierd and sloppy way of basically making sure there are only N values possible in the 
output. Apply this to a sine wave with a low value of N and you get sort of stacked squares. 
Secondly it has a gross digital clipping amplifier. Basically signal is the clamp of input times
param. APply this to a sine wave and turn it up and you get pretty much a perfect square.
Combine them for grunky grunk noise. Dumb, but fun. Here's a sample patch.

![Example Bitulator Patch](doc/Bitulator.png)

## License

Copyright © 2017  Paul Walker

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


