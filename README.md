# Lars-2.0

Lars-2.0 is a Windows-compatible VST3 plugin and standalone application designed for drum demixing in music tracks. Developed by students at Politecnico di Milano, it intelligently separates drum elements from the rest of a song, making remixing, production, sampling, and musical practice easier. 

This is the second version of Lars — the first drum demixing plugin created by students of Politecnico di Milano.


## 🎵 Description

Lars-2.0 leverages advanced audio processing and machine learning techniques to extract drum components from music tracks. It is intended for producers, musicians, DJs, and researchers who want to easily isolate drums, either as a VST3 plugin or a desktop application.

---

## 🚀 Main Features

- Drum demixing: separate drums from the full mix
- VST3 plugin and standalone desktop application support
- Intuitive graphical user interface
- Offline processing
- Compatible with Windows (tested on Windows 11)
- Export separated tracks

---

## 📸 Screenshot

![Drum demixer](https://github.com/user-attachments/assets/5be14650-e0a6-4a70-b67a-46c67f4fb934)

---

## ⚙️ Requirements

- **Operating system:** Windows (tested on Windows 11)
- **Compatible DAWs:** (for VST3, e.g., Ableton Live, FL Studio, Reaper, etc.)
- **Dependencies:**
  - CMake ≥ 3.30.3 (tested with 3.30.3 and 3.31.6)
  - Additional libraries: libtorch 2.6.0+cpu

---

## 🛠️ Installation

### Clone the repository

```bash
git clone https://github.com/FranciscoMessina00/Lars-2.0.git
````

> 🛑 **Note about Git LFS (Large File Storage)**
> This repository uses **Git LFS** to store large model files (e.g., `mdx23c.pt`).
> If you encounter an error like:

```
Error downloading object: models/mdx23c.pt [...] This repository exceeded its LFS budget.
```

It means the GitHub LFS quota has been exceeded.

#### ✅ Workaround: Manually download the model

1. Manually download the model file `mdx23c.pt`, `mdx23c_capstone.pt`, and `mdx23c_inst_sep.pt`.
2. Create the `models/` directory inside the cloned repository if it doesn't already exist:

   ```bash
   mkdir -p Lars-2.0/models
   ```
3. Move the downloaded model files into that folder, for example:

   ```
   Lars-2.0/models/mdx23c.pt
   ```

After that, everything should work as expected.

#### **Download Libtorch**
   Grab the CPU-only Libtorch release (v2.6.0) ZIP from:  
https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-2.6.0%2Bcpu.zip

#### **Extract folder**
- Locate the downloaded file (e.g. `libtorch-win-shared-with-deps-2.6.0+cpu.zip`).  
- Extract it (right-click → **Extract All…**, or use your preferred tool) into a temporary folder.  
- Inside the temporary folder you should now have a folder named `libtorch` containing subfolders like `bin/`, `include/`, and `lib/`.

#### **Organize in Your Project**
In your project root, create a `libs/` folder (if it doesn’t already exist) and move the extracted `libtorch` directory into it.

### Build (VST3/Standalone)
```bash
cd Lars-2.0
cmake -S . -B build
```

This will automatically install JUCE 8.0.2. It will also set up the testing pipeline if the user chooses to use it—note that we have not configured or used the testing pipeline ourselves.

- After the first build, compile the desired format in Release mode.
- After the compilation, the VST3 plugin and standalone software will be in `.\build\plugin\SlothPlugin_artefacts\Release`.
- Move the VST3 folder to `C:\Program Files\Common Files\VST3` and the Standalone directory to your preferred location (don't change the relative position of the internal files).

To make the VST3 work properly, libtorch's DLLs inside `.\libtorch\lib\` must be placed inside the `System32` folder, or the path to `.\libtorch\lib\` must be added to the system's global PATH environment variable. We acknowledge this is not the ideal installation process, but currently, it is the only way we have managed to get the VST3 working on Windows. There does not seem to be an easy solution for making libtorch work with the VST3 format. For more details, do not hesitate to contact us. For the standalone version, it is sufficient to leave the DLLs inside the same folder as the executable file.

Furthermore, the `.pt` files, which are the separation models, must be in the same folder as the VST3/standalone. If the program gives an error in finding the models it could be that the download of the repository failed to save them properly. If so, try to download them manually, place them inside the `.\models` folder and recompile the whole project.

We are planning to develop an installer to make the installation process simpler and faster.

---

## ▶️ Usage

- **VST3 Plugin:** Load "Lars-2.0" as an audio effect in your favorite DAW, load a track, and adjust the demixing parameters.
- **Standalone:** Launch the application, load the audio file, and start the drum demixing process. Export the separated tracks when finished.

---

## ⚙️ UI and Functions

- Load an audio track in WAV, FLAC, or MP3 format in the upper section of the device by drag-and-drop or by clicking the upload button. The output files will be in WAV format. Two waveforms will be displayed: green for the left channel and red for the right channel.
- Press the play/stop button to play or pause the audio file you uploaded.
- Drag the transport bar to navigate through the file. Click anywhere on the waveform to jump to that point, or double-click to jump to the beginning of the file. Playback will automatically loop when the transport bar reaches the end of the file.
- Press the divide button to start audio separation. You can still listen to and navigate the file during the separation process.
- When the process is finished, the separated tracks will be loaded in the bottom part of the device. The same transport controls are available for the visualized separated track. To change the displayed track, press one of the available track buttons.
- You can save the separated files by dragging one of the track buttons to your desired location (even directly into an audio track in your DAW), or you can save all tracks in a single location by pressing the download button.
- You can switch the separation model by using the drop-down menu. For more details about the separation models, please refer to the file in the repo _L13_report.pdf_.

---

## 🤝 Contributing

1. Fork the repo and create a `feature/your-feature-name` branch.
2. Commit your changes.
3. Open a Pull Request.
4. Follow the coding and documentation guidelines.
5. Report bugs or suggest improvements via Issues.

---

## 📜 License

This project is released under the MIT license. See the [LICENSE](LICENSE) file for details.

---

## 👤 Authors & Contacts

- Francisco Messina — [GitHub](https://github.com/FranciscoMessina00), franc.mess@gmail.com
- Giuliano Galadini — [GitHub](https://github.com/Zulino), giulianogaladini@gmail.com

---

## 🙏 Acknowledgements

- The Audio Programmer community for support on the Discord server and all the open-source material available.
- JUCE framework
- ZFTurbo for making the drum separation model architectures available.
- Politecnico di Milano for the opportunity to work on this stimulating project.

---
