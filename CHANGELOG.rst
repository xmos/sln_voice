XCORE-VOICE change log
======================

2.2.0
-----

  * TODO: Add entries here

2.1.0
-----

  * ADDED: Mic aggregator app that bridges between 16 mics and TDM16 slave or USB Audio
  * CHANGED: Updated submodule fwk_io on to version 3.1.0 to add support for TDM16 slave tx and 16ch mic_array

2.0.0
-----

  * ADDED: All speech recognition example deigns now use Sensory TrulyHandsfree speech recognition library.  
  * ADDED: Low-power far-field dictionary (low-power FFD) example design using wakeword to exit standby mode.  
  * ADDED: Mandarin model to far-field dictionary (low-power FFD) example design demonstration.
  * ADDED: Support for fast flash library which increases flash reading throughput by as much as 70%.
  * MOVED: Audio pipelines relocated to modules to allow easier re-use.
  * FIXED: Example designs now indicate when evaluation period has expired.
  * FIXED: Numerous minor bug fixes.  

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
