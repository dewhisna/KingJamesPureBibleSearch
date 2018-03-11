How to find the ALSA device:

$ pacmd list | grep "\.monitor"
name: <alsa_output.pci-0000_01_00.1.hdmi-stereo.monitor>
name: <alsa_output.pci-0000_00_1b.0.analog-stereo.monitor>
        alsa_output.pci-0000_01_00.1.hdmi-stereo.monitor/#0: Monitor of HDA NVidia Digital Stereo (HDMI)
        alsa_output.pci-0000_00_1b.0.analog-stereo.monitor/#2: Monitor of Built-in Audio Analog Stereo


$ parec -d <monitor> --file-format=wav output.wav

$ parec --list-file-formats

---or---

$ pacat --record -d alsa_output.pci-0000_00_1b.0.analog-stereo.monitor > dump.raw
$ sox -t raw -r 44100 -e signed-integer -L -b 16 -c 2 dump.raw output.wav

