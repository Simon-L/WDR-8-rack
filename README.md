# WDR-8

### Wave Digital Rhythm drum machine, based on TR-808

## WDR-8 Snare

![](https://nextcould.roselove.pink/apps/files_sharing/publicpreview/NzHELScjLKEFeQH?x=1920&y=613&a=true&scalingup=0)

Original schematics:  
https://nextcould.roselove.pink/apps/files_sharing/publicpreview/rkeyCqN837TBJF2?x=1920&y=613&a=true&file=fullsnareschematics.png&scalingup=0

W.N = White noise  
Trigger comes from top left  

This module uses WDF models for :  
- A simplified envelope generator (Q47) with adjustable R and C compononents
- The low shell resonator (IC 14, left) with the shaper at the input
- The high shell resonator (IC 14, right)
- A very simplified diode clipper used as the base for the VCA (Q48)

The high pass filter (Q49, C66, C67, R201, R202) is not simulated, instead a matching 2-pole Butterworth HPF from ChowDSP is used.  
IC 13 has not been analysed and is considered ideal, the Volume knob has this purpose.

The shell resonators are the work of Jatin Chowdhury, as part of the WaveDigitalFilters repo: https://github.com/jatinchowdhury18/WaveDigitalFilters/tree/master/TR_808/SnareResonator/src  

## Design

Fonts: [Olyford Semi Bold](https://www.fonts.com/font/nicolassfonts/olyford/semi-bold) and [Noto Sans](https://fonts.google.com/noto/specimen/Noto+Sans)

## Acknowledgments
Many thanks to Jatin Chowdhury, Paulbacon, Steve Norgate, Eric Archer, Kurt J. Werner  

https://norgatronics.blogspot.com/2021/11/sd-8081-tuning.html  
http://www.ericarcher.net/wp-content/uploads/2014/07/808-svc-man.pdf  
https://stacks.stanford.edu/file/druid:jy057cz8322/KurtJamesWernerDissertation-augmented.pdf  
https://github.com/jatinchowdhury18/WaveDigitalFilters  
 
## License
GPL v3
