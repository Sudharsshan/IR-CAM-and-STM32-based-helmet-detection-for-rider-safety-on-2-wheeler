# A Vision-Based Helmet Detection Approach for Two-Wheeler Rider Safety

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B-blue.svg)
![Framework](https://img.shields.io/badge/framework-STM32Cube-orange.svg)
![ML Framework](https://img.shields.io/badge/ML%20Framework-TensorFlow%20Lite-brightgreen.svg)

An intelligent, on-device system designed to enforce helmet use and enhance road safety for two-wheeler riders. This project was developed as a final-year B.Tech project in Electrical and Electronics Engineering at CHRIST (Deemed to be University), Bengaluru.

## 📖 Introduction

[cite_start]Motorcycle fatalities are strongly correlated with the non-usage of helmets[cite: 649]. [cite_start]This project presents an embedded, on-device Machine Learning system that detects helmet usage and enforces a safety interlock on the motorcycle's ignition circuit[cite: 651].

[cite_start]The core of the system is an **STM32H7 microcontroller** running a custom TensorFlow Lite model trained on **Edge Impulse**[cite: 618, 692]. [cite_start]It uses an infrared (IR) camera to ensure reliable detection day or night[cite: 620, 621]. [cite_start]If a helmet is not detected, the system initiates a 3-minute warning before safely disabling the ignition via a relay[cite: 625, 626]. All processing is done locally, ensuring **low latency** and **100% privacy**.

## ✨ Key Features

-   **On-Device ML Inference**: All helmet detection is performed directly on the microcontroller. [cite_start]No cloud connection is needed, ensuring privacy and real-time performance[cite: 627, 695].
-   [cite_start]**Infrared (IR) Imaging for Low-Light Robustness**: An array of IR LEDs is automatically activated in low-light conditions, guaranteeing reliable detection 24/7[cite: 621, 900].
-   [cite_start]**Fail-Safe Ignition Control**: A relay is connected to the motorcycle's kill-switch line, which defaults to an "off" state, preventing ignition if the system fails or loses power[cite: 705, 989].
-   [cite_start]**Humane Warning System**: If a rider is detected without a helmet, a 3-minute countdown is displayed on an LCD, giving the rider ample time to comply before ignition is disabled[cite: 625].
-   [cite_start]**Deterministic & Low-Power Operation**: The system performs a check every 15 minutes using the Real-Time Clock (RTC), balancing safety enforcement with minimal power consumption[cite: 625, 901].
-   [cite_start]**Retrofit-Friendly Design**: Designed as a generic module that can be fitted to any two-wheeler, whether it has an Internal Combustion Engine (ICE) or is an Electric Vehicle (EV)[cite: 902].

## ⚙️ System Workflow

The project followed a structured development methodology, from initial requirements to final validation.

## 🛠️ Hardware & Software

### Hardware Components
The prototype was built using the following key components:
-   [cite_start]**Microcontroller**: WeAct Studio STM32H723VGT6 Board (ARM Cortex-M7 @ 550 MHz) [cite: 1296, 1297]
-   [cite_start]**Camera**: OV2640 Infrared Camera Module with DVP interface [cite: 918]
-   [cite_start]**Power Supply**: 12V 120W SMPS (acting as a vehicle battery) [cite: 1308, 1314]
-   [cite_start]**Voltage Regulation**: 10-60V to 5V 3A Waterproof Buck Converter [cite: 1319, 1320]
-   **Actuator**: 5V Single-Channel Relay Module
-   **Display**: I2C LCD Display
-   **Lighting**: IR LED Array

### Software Stack
-   [cite_start]**ML Development Platform**: [Edge Impulse Studio](https://www.edgeimpulse.com/) for dataset management, model training, and quantization[cite: 1223].
-   [cite_start]**Firmware IDE**: [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) for C/C++ firmware development and debugging[cite: 1226].
-   [cite_start]**ML Runtime**: [TensorFlow Lite for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers) for executing the model on the STM32H7[cite: 1230].
-   [cite_start]**System Modeling**: MATLAB & Simulink for pre-deployment validation of state logic and timing[cite: 1233].

## 🚀 Getting Started

### Prerequisites
-   [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) installed on your computer.
-   The hardware components listed above.

### Installation & Setup
1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/Sudharsshan/IR-CAM-and-STM32-based-helmet-detection-for-rider-safety-on-2-wheeler.git](https://github.com/Sudharsshan/IR-CAM-and-STM32-based-helmet-detection-for-rider-safety-on-2-wheeler.git)
    ```
2.  **Open in STM32CubeIDE:**
    -   Launch STM32CubeIDE.
    -   Go to `File > Open Projects from File System...`.
    -   Select the cloned repository folder and import the project.
3.  **Build and Flash:**
    -   Connect your STM32 board to your computer.
    -   Build the project using the "Build" button (hammer icon).
    -   Flash the firmware to the board using the "Run" button.

## 📈 Results

The final INT8 quantized model achieved excellent performance on the test dataset and in real-world validation:
-   [cite_start]**Detection Accuracy**: **99.7%** in both daylight and low-light (IR-assisted) conditions[cite: 1021, 1022].
-   [cite_start]**F1-Score**: **92.9%** (a strong balance of precision and recall)[cite: 1021].
-   [cite_start]**Inference Latency**: A median of **145 ms** on the STM32H7 core[cite: 1025].
-   [cite_start]**Reliability**: The system completed a 48-hour continuous run with **0 missed detection cycles**[cite: 1027].

## 🔮 Future Work

This project serves as a strong foundation. Future enhancements could include:
-   [cite_start]**Rider Identification**: Integrate a lightweight face recognition model or NFC/RFID to link helmet compliance to specific, authorized riders[cite: 1157].
-   [cite_start]**Accident Detection & SOS Alerts**: Add an IMU (accelerometer/gyroscope) to detect crashes and automatically send GPS coordinates to emergency services[cite: 1161].
-   [cite_start]**V2X Integration**: Incorporate a communication module (e.g., LoRaWAN) to broadcast anonymous safety data to smart traffic infrastructure[cite: 1165].

## 🤝 Contributing

Contributions, issues, and feature requests are welcome! Feel free to check the [issues page](https://github.com/Sudharsshan/IR-CAM-and-STM32-based-helmet-detection-for-rider-safety-on-2-wheeler/issues).

## 📜 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

-   This project was submitted in partial fulfillment of the requirements for the degree of Bachelor of Technology at **CHRIST (Deemed to be University)**.
-   Special thanks to our project guide, **Dr. [cite_start]Srihari Gude**, for his constant monitoring and encouragement[cite: 602, 603].
