# Hematuria monitor

## An open source solution for management and automation of continuous bladder irrigation using off-the-shelf consumer electronics.
Continuous bladder irrigation (CBI), or bladder washout, is a procedure to prevent blood clot formation after urological procedures (prostatic resection, kidney stone removal, et cetera). Most CBI is conducted with a Foley catheter and is monitored continuously by a nurse or aide to ensure that the flow rate through the bladder is sufficient. If more blood is seen, the flow rate is typically increased; drip rate is decreased as the color resolves.

The administration of CBI requires intensive nursingsupport for proper administration and appropriate physician ordering for safety. [citation](https://digitalcommons.wustl.edu/cgi/viewcontent.cgi?article=4392&context=open_access_pubs) Unofrtunately, complete reliance on manual monitoring is falliable and we believe the process can be augmented through digitized color recognition and wireless updates to the CBI-supervising provider.

## Description

This open hematuria monitor (hereby OHM), fully assembled, is an Arduino-controlled device that measures saline flow from the Foley catheter input, through the bladder, and to the outflow collection bag with a load sensor (HX711), and analyzes the absorptivity of outflow irrigate at multiple wavelengths with a visible-light spectrophotometer (AS7262) to non-invasively detect hemoglobin (oxygenated or deoxygenated) in the irrigate. It integrates these two datasets to estimate a variety of statistics about the CBI process, and projects integrated data to a plot.ly webserver with an onboard WiFi chip (ESP8266). OR it can send updates to a custom-built local radio pager using a 315mHz (or of any bandwidth chosen) transmitter/receiver combo. 

<p float="left">
  <img src="https://github.com/malyalar/auto-hematuria-monitor/master/gallery/IMG_20200705_122330_crop.jpg", height="210" />
  <img src="https://github.com/malyalar/auto-hematuria-monitor/blob/master/gallery/IMG_20200607_172259_crop.jpg", height="210" />
  <img src="https://github.com/malyalar/auto-hematuria-monitor/blob/master/gallery/IMG_20200607_173534_crop.jpg", height="210" />
</p>


Currently, the device can measure and display estimates of:
- hematuria grade (or blood volume loss per minute, assuming hematocrit of 40);
- total saline used, and rate of collection bag filling;
- [PENDING] it can display these parameters to a Blynk webserver that can be accessed with a cellphone.
- [PENDING] it can send this data to a wireless display/pager over a distance of 

The device also can display alarms when:
- the collection bag is nearing capacity, or when the inflow saline is running out;
- if blood loss (as volume or hemoglobin) is excessive, and;
- [PENDING] send an SMS/email (if configured to do so) to your nurses'/residents' phones.

Future functionality for the device will include motor control to automatically titrate catheter inflow as the estimated rate of blood loss increases, although this is challenging since there may be some clinical considerations other than the observed rate of hematuria that may influence the chosen irrigation rate. Verification and validation processes for controlling the flow rate are also more intense and become harder to justify with respect to medical device regulations.

## Assembly

CAD files are *mostly* parametric and can be modified to accommodate larger/smaller Arduino/other microprocessor units, buttons, displays, etc. The .stl files used in our device build are provided as well, but work only with the specific buttons, switches, and displays we've purchased. Current calibration based on absorbance through a Baxter Y-type TUR/Bladder Irrigation Set.

### Bill of Materials
- HX711 amplifier circuit: $8
- HX711 10kg load cell: $10
- 240x240 TFT 1.53" display: $25
- Arduino (any Arduino type reprogrammable microcontroller will work; a mini-style microcontroller will be cheaper): $10
- Momentary buttons: $5
- AS7262 visible light sensor: $25
- ESP8266 WiFi breakout: $12
- voltage level regulator (for ESP8266): $10
- Voltage stepper chip (for ESP8266): $5
- Miscellaneous jumper cables, usb male/female ends (these are quite varied, but most commercial usb cables can be spliced for the purposes of this project.): $10
- **TOTAL : $130 before tax**; not including 3d filament, 3d printer, misc. consumables such as solder, etc.


## Operation
Every loop (operating at approximately 3s intervals) the load sensor tracks weight and change in weight, and the spectrophotometer tracks absorbance at six spectral channels, spaced at 450, 500, 550, 570, 600 and 650 nm. Hemoglobin is the primary analyte of interest, and while commercial spectrophotometry-based blood-testing devices (for example, pulse oximeters and non-invasive hematocrit asPsessors) typically use the IR or near-IR spectrum (taking advantage of the massive divergence in absorbance spectra between deoxygenated and oxygenated hemoglobin), the present device estimates hemoglobin concentrations within the visible spectrum only. (Purchasing an alternate, equivalently expensive spectrophotometer breakout will give access to the IR and NIR ranges.) The [AS7262 6-channel Visible Spectral ID device](https://cdn.sparkfun.com/assets/f/b/c/c/f/AS7262.pdf) comes with lifetime-calibrated sensing with minimal drift over time or temperature. The quality of the readouts from the spectrophotometer is apparently dependent on the [gain (1x, 3.7x, 16x, and 64x) and integration time (maximum 182ms)](https://adafruit.github.io/Adafruit_AS726x/html/class_adafruit___a_s726x.html#aefb04c53faed2c942ce44297acaea2a7) selected for the device, as detailed [page 7 of the AS726x-iSPI Evaluation Kit GUI user guide](https://ams.com/documents/20143/36005/AS726x_UG000340_4-00.pdf/98588d96-a807-d8ec-5251-370a0be3069b). 

The program sketch operates as follows. Taking ten measurements a second at the load sensor and ten measurements a second at the spectrophotometer, the device builds a moving average of five measurements for both, and compares to the moving averages 1 second ago to calculate rates of change. Using the beer-lambert law and a preset calibration curve using Baxter Y-type clear tubing (you will have to recalibrate if using different tubing through the spectrophotometer device), the sketch multiplies current absorbance (correlated to a concentration in mol(of HgB)/L) by a irrigate flow rate (mL/sec) to get a flow rate of amount fo HgB (mol/sec). When mols/sec HgB exceeds a predetermined amount (adjustable in GUI), an alert is sent to the supervising nurse/resident to adjust irrigation flow.

## Blood content determination
Using the AS7262 or any consumer vis-light spectrophotometer module to estimate hgb content and concentration at the outflow tube brings several challenges. Hemoglobin and deoxyhemoglobin have different absorption spectra and molar extinction coefficients, roughly equivalent at certain points in the vis-light spectrum measured by the AS7262 (i.e. isobestic), but vastly different at other points. White LEDs used to test transmittance through the outflow tubing have different spectral power distributions, making no algorithm for HgB determination not necessarily a one-build-fits-all solution. For example, two different LEDs both considered 'white-light' LEDs may transmit very different amounts of purple-wavelength light, such that using absolute values of purple light absorbance at the photodiode useless between two builds. Thus, every build of this device may have to undergo calibration with a 'blank' outflow tube of saline to ensure that blood content estimates are meaningful and useful for monitoring CBI (if not necessarily perfectly accurate). If built to specification, the device has functionality on button press to go into a 'calibration mode' to allow the user to prepare and calibrate with their own 'blank' saline-filled tubing.

![](img/2020-07-12-21-24-36.png)

important reference: [Development of white LED illuminants for colorimetry and recommendation of white LED reference spectrum for photometry](https://iopscience.iop.org/article/10.1088/1681-7575/aacae7) Image not from this paper.


The error at the photodiode panel, with units reported in uW/cm^2, is +/-12%. 

Furthermore, a clip-on design cannot reasonably be light-isolated without excessively taping the tubing or extending the spectrophotometer shroud. Part of the hemoglobin determination calculation described herein is intended to correct for the slight differences in ambient light between night and day or between well vs. poorly lit rooms, which may bleed into the photodiode shroud.

![](img/2020-07-11-00-19-59.png)

![](img/2020-07-11-12-05-16.png)
![](img/2020-07-11-12-05-39.png)


## Determining hemoglobinuria quantitatively with no lookup table

