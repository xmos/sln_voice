XCORE-VOICE change log
======================

1.0.0
-----

  * ADDED: Improved documentation 
  * ADDED: Speech recognition porting example design
  * FIXED: USB PIDs changed to 0x4000 for FFVA and 0x4001 for FFD
  * FIXED: Speech recognition model no longer in the filesystem
  * FIXED: Several Linux USB audio bugs
  * REMOVED: audiomux example design

0.21.0-beta.0
-------------

  * ADDED: Improved Interference Cancellation using Voice-to-Noise Ratio estimator  
  * ADDED: Low-power mode to far-field dictionary example design
  * ADDED: USB DFU to far-field voice assistant example design
  * ADDED: Optional AEC fixed-delay to far-field voice assistant example design

0.20.0
------

  * ADDED: Audio response playback
  * ADDED: Wanson 20220923 model update
  * ADDED: Documentation updates
  * FIXED: USB audio device is now recognized on Windows
  * FIXED: Applications moved to examples folder
  * FIXED: Sample rate conversion from 48k fixed

0.12.0
------

  * ADDED: Audio playback to FFD application
  * ADDED: First draft of documentation
  * KNOWN ISSUE: USB audio device is not recognized on Windows  (This does not effect FFD)

0.10.0
------

  * ADDED: FFD demo using OLED display
