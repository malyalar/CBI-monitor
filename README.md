# Hematuria monitor

## An open source solution for management and automation of continuous bladder irrigation using off-the-shelf consumer electronics.
Continuous bladder irrigation (CBI), or bladder washout, is a procedure to prevent blood clot formation after urological procedures (prostatic resection, kidney stone removal, et cetera). We believe the process can be augmented through digitized color recognition and wireless updates to the CBI-supervising provider.

## Description
This hematuria monitor ("HM") is an Arduino-controlled device that measures saline flow from the Foley catheter input, through the bladder, and to the outflow collection bag with a load sensor (HX711), and analyzes the absorptivity of outflow irrigate at multiple wavelengths with a visible-light spectrophotometer (AS7262) to non-invasively detect hematuria in the irrigate. It integrates these two datasets to estimate a variety of statistics about the CBI process. In it's current implementation, it can send updates to a custom-built local radio pager (enclosure CAD available, but schematics pending) using a 315mHz transmitter/receiver combo. 

<p float="left">
  <img src="https://github.com/malyalar/auto-hematuria-monitor/blob/master/gallery/IMG_20200705_122330_crop.jpg", height="210" />
  <img src="https://github.com/malyalar/auto-hematuria-monitor/blob/master/gallery/IMG_20200607_172259_crop.jpg", height="210" />
  <img src="https://github.com/malyalar/auto-hematuria-monitor/blob/master/gallery/IMG_20200607_173534_crop.jpg", height="210" />
</p>


Currently, the device can measure and display estimates of:
- estimates hematuria grade;
- total saline used, and rate of collection bag filling;

Future functionality for the device may include motor control to automatically titrate catheter inflow as the estimated rate of blood loss increases, although this is challenging since there may be some clinical considerations other than the observed rate of hematuria that may influence the chosen irrigation rate. Verification and validation processes for controlling the flow rate are also more intense and become harder to justify with respect to medical device regulations.

## Assembly
Assembly guides are pending. 

CAD files are parametric and can be modified to accommodate larger/smaller Arduino/other microprocessor units, buttons, displays, etc. The .stl files used in our device build are provided as well, but work only with the specific buttons, switches, and displays we've purchased. Specific electrical schematics are available but subject to change. Current calibration is based on absorbance through a Baxter Y-type TUR/Bladder Irrigation Set.

## Operation
Every loop (operating at approximately 3s intervals) the load sensor tracks weight and change in weight, and the spectrophotometer tracks absorbance at six spectral channels, spaced at 450, 500, 550, 570, 600 and 650 nm. Hemoglobin is the primary analyte of interest, but the device overall functions mainly as a colorimeter. The chosen spectral sensing unit for this device, the [AS7262 6-channel Visible Spectral ID device](https://cdn.sparkfun.com/assets/f/b/c/c/f/AS7262.pdf) comes with lifetime-calibrated sensing with minimal drift over time or temperature.

The program sketch operates as follows. Taking ten measurements a second at the load sensor and ten measurements a second at the spectrophotometer, the device builds a moving average of five measurements for both, and compares to the moving averages 1 second ago to calculate rates of change. Using a preset lookup table on serial dilutions of blood in Baxter Y-type clear tubing (you will have to recalibrate the lookup table if using differently colored tubing with the spectrophotometer device), the sketch estimates a hematuria grade from the current absorbance at purple wavelengths.
