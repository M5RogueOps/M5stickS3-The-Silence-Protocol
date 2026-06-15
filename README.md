# 📟 The Silence Protocol

**A fully playable, hardware-integrated text adventure built exclusively for the M5Stack StickS3.**

> *The year is 2028. The "Silence"—a highly aggressive, decentralized malware—has crippled the global communication grid. Heavy-duty servers and standard operating systems are entirely compromised. Operating out of the Ethical Hackers Den, you are humanity's last line of defense. Armed only with a microcontroller, you must descend into the North-Eastern Hub and manually reboot the core.*

**The Silence Protocol** pushes the M5Stack StickS3 to its limits, transforming it from a simple dev board into a ruggedized, interactive cyberdeck. This isn't just a text game—it requires you to physically move, listen, and hack your way to victory.

---

## ✨ Features

* **Authentic CRT Aesthetics:** Custom-coded display engine featuring muted phosphor color palettes, persistent hardware scanlines, and VHS-style screen tearing/static effects during combat or damage.
* **Persistent Cyberdeck HUD:** Real-time tracking of your current HP (Health Points) and location within the facility. 
* **Hardware-Driven Minigames:**
  * **Frequency Sync:** A timing-based visual hack requiring split-second button presses to bypass security cameras.
  * **Kinetic EMP:** Rapidly shake the physical StickS3 (utilizing the internal IMU accelerometer) to charge an EMP burst and defeat heavy drones.
  * **Gyroscopic Balance:** Hold the device perfectly flat to carefully cross severed power relays without falling.
  * **Audio Stealth:** The game activates the StickS3's built-in PDM Microphone. You must remain completely silent in the real world to avoid detection by automated QA Scanners.
  * **SSH Binary Handshake:** A rapid-fire binary input memory game.
  * **Firewall Overload:** A frantic button-mashing sequence to overpower the final core firewall.
* **Real-World WiFi Immersion:** Upon reaching the core, the game scans your actual physical environment and targets your real local WiFi networks for payload injection.
* **Procedural Gameplay:** The ventilation maze is procedurally generated upon every new playthrough.
* **Persistent Save Data:** The ESP32's non-volatile flash memory tracks your discovered endings. Find the Main Victory and the Pyrrhic Victory to unlock a hidden Developer Mode on the title screen.

---

## 🛠️ Hardware Requirements

* **M5Stack StickS3** (Compatible with standard and S3 variants, provided the screen resolution matches).
* USB-C Cable for flashing.

---

## 💻 Software Dependencies

To compile and upload this game via the Arduino IDE, ensure you have the following libraries installed:

1. **M5Unified** (Install via Arduino Library Manager)
2. **WiFi.h** (Built into the ESP32 Board Manager)
3. **Preferences.h** (Built into the ESP32 Board Manager)

**Setup Instructions:**
1. Open the Arduino IDE and select your M5Stack board.
2. Ensure your `Upload Speed` is set to `1500000`.
3. Copy the `silence_protocol.ino` code into your sketch.
4. Compile and upload to your device.

---

## 🎮 How to Play

The game relies entirely on the StickS3's two physical inputs:
* **[BtnA]**: The main front button (M5 Logo). Used for primary selections, hacking, and advancing text.
* **[BtnB]**: The side button. Used for secondary selections or alternate paths.

Pay attention to the text prompts, as the game will frequently ask you to use the device physically (shaking, balancing, or remaining quiet) to survive. Keep an eye on your HP—if it hits 0, the Silence claims you.

---

## 🛡️ About the Creator

**The Silence Protocol** was created and engineered by **M5RogueOps**. 

* 🐙 **GitHub:** [M5RogueOps](https://github.com/M5RogueOps)
* 🌐 **Community & Blog:** [ethical Hackers Den](https://www.ethicalhackersden.org)

Join the Den to explore more hardware hacking, microcontroller deployments, and embedded electronics projects. 

---
*License: MIT License - Free to modify, hack, and deploy.*
