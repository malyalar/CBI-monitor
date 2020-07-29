## Non-invasive real-time spectrophotometric quantification of ex-vivo hemoglobinuria with one-step calibration.

### Current device implementation and justification

In the current implementation of the hematuria monitor, a white LED is mounted opposite to an AS7262 visible light spectrophotometer module, passing *incident* light through a catheter outflow tube clipped inside the spectrometer shroud, resulting in a reading of a quantity of *transmitted* light at the photodiode module. 

Ordinarily, in a prototypical spectrophotometer, concentration for solutes in solvent (here, blood in saline) is calculated using the Beer-Lambert law: 

$A=log_{10}(I_{o}/I_{t})=\epsilon * c * l$, where: 

$A$ is **absorbance** (here re-expressed as the log of the ratio between incident and transmitted light quantities), $l$ is the **length of the path** traveled by the incident light before reaching the photodiode, $c$ is the **concentration of the solvent** in $M$, and $\epsilon$ is the **molar extinction coefficient** of the solute for light at a particular wavelength in $M^{-1}cm^{-1}$.

If the molar extinction coefficient, path length, and transmittance/absorbance for a solute is known, concentration can be calculated directly (assuming no confounding second solutes are present). Otherwise, a lookup table associating observed absorbances with particular concentrations, i.e. a calibration curve, can be used to inter- or extrapolate solute concentration. In the current embodiment however, there are two concerns that disqualify the use of these more simple methods of quantification. 

**The first problem:** as some sections of outflow catheter tubing leading to the outflow bag must necessarily be exposed to light, and because the spectral power distribution of white LEDs is not always well characterized, and moreoever varies with the chosen LED, its drive current, individual efficiency, manufacturing process, and attached resistor, it is difficult to physically control appropriately for the original incident quantity of light between seperate builds of the device. The following method of quantification is intended to avoid an overly extensive calibration process between device builds.

**The second problem:** the determination of concentration of hemoglobin content in tubing is confounded by the different absorption spectra of oxygenated and deoxygenated hemoglobin (hereafter Hb-O2 and Hb, respectively). However, the ratio of Hb-O2 and Hb, i.e. SpO2, is not already known. In order to control for this confound, two *isosbestic* points in the visible spectrum for Hb-O2 and Hb are used to calculate the absorbance of hemoglobin. 

The method herein still requires some determination of the average ratio of incident light strength at the chosen observed wavelengths. However, this is a one-step simple calibration process and is more robust to changes in ambient lighting, variable resistance and irradiance at or from the LED, and thus does not require each user to construct their own calibration curve with serial dilutions. 

### Method of quantification of hemoglobinuria

1) $C_{hgb-total} = C_{Hb} + C_{Hb-O^{2}}$

2) $ \%\ transmittance = I_{t}/I_{o} * 100\%$

3) $Absorbance = -log_{10}(\%\ transmittance)$

4) $Absorbance =\epsilon * concentration * path\ length$

Where $I_{o}$ describes the quantity of light *transmitted* from the LED mounted opposite the photodiode module entering the sample, and $I_{t}$ describes the quantity of light exiting the sample and registering on the photodiode.

First recognize that the total absorbance at a given wavelength, here chosen at 500nm ("blue"), is the sum of the dual absorption of HbO2 and Hb. 

$A_{500nm} = \epsilon_{HbO2-500nm} * c_{HbO2-500nm}*l + \epsilon_{Hb-500nm} * c_{Hb-500nm}*l_{500}$

Because 500nm is roughly an isosbestic point for the absorption spectra for HbO2 and Hb, [see citation](https://omlc.org/spectra/hemoglobin/summary.html), $\epsilon_{HbO2-500nm}$ and $\epsilon_{Hb-500nm}$ are equivalent (20932.8 $\simeq$ 20862.0). Absorbance can also be re-expressed in terms of log transmittance calculated from the spectrophotometer output. Therefore:

$log_{10}(\frac{I_{o}}{100*I_{t}}) = (\epsilon_{Hb\ total-500nm} * c_{Hb\ total})*l_{500}$

$I_{t}$, $\epsilon_{Hb\ total-500nm}$, and $l_{500}$ are known from reference values or the spectrophotometer result, leaving $I_{o}$ as the remaining unknown before determining the total concentration of hemoglobin. While ordinary spectrophotometric or colorimetric devices can assume a known $I_{o}$, the construction of the hematuria monitor in its current implementation leaves $I_{o}$ as variable in different builds/environments. Without reliable incident light data due to confounding from ambient light, variable resistance at the LED, and other factors discussed in the introduction, a system of equations can be built to fix $I_{o}$ using the same formula at another wavelength of light, here chosen at 570nm ("green").  

$log_{10}(\frac{I_{o}}{100*I_{t}}) = (\epsilon_{Hb\ total-570nm} * c_{Hb\ total})*l_{570}$

$log_{10}(I_{o}) = (\epsilon_{Hb\ total-570nm} * c_{Hb\ total})*l_{570} + log_{10}({100*I_{t}})$


570nm is another roughly isosbestic point for HbO2 and Hb (44496 $\simeq$ 45072) in the visible spectrum.

At this stage, if $I_{o}$ can be set equal between the chosen wavelengths, a system of equations can be built to isolate for total concentration of hemoglobin. Ratios of transmittance of light from a white LED at 500nm and 570nm are easy to determine. Through a blank tube, averaged over ten measurements, the ratio in our current build of the spectrophotometer clip is approximately 1.04 (1.04153); i.e. the counts per power per unit area generated by the light hitting the photodiode for 500nm is 1.04 times greater than it is for 570nm transmitting light through a blank control tube, in our specific build of the device. 

If (5) $I_{o,\ 500nm} = 1.04*I_{o,\ 570nm}$, then:

(6) $[(\epsilon_{Hb\ total-500nm} * c_{Hb\ total})*l_{500} + log_{10}(100*I_{t,\ 500nm})] = 1.04*[(\epsilon_{Hb\ total-570nm} * c_{Hb\ total})*l_{570} + log_{10}(100*I_{t,\ 570nm})]$

Where $c_{Hb\ total}$ can be trivially isolated.

It may appear that path length $l$ can be factored out, but the path length of the incident light is dependent on the specific wavelength of the light, as different light wavelengths have different refractive indices. The physical  distance between source and  detector cannot be used directly  as the path length because the light  inside a medium gets reflected and doesn’t pass through a straight path. Optical distance $T_{f}$ is defined as the product of refractive index of medium, $n$, and the geometric distance travelled by the light, $d$.

$T_{f}=n*d$

Isolating for $C_{hb\ total}$, and replacing path lengths corrected for refraction [citation](https://www.osapublishing.org/ao/abstract.cfm?uri=ao-45-12-2838) and molar extinction coefficients with actual values:



$C_{hgb-total}=\frac{log(I_{t,\ 500nm}/(1.04*I_{t-570nm}))}{(44784*T_{f-500nm}-20897*T_{f-570nm})}$

x-axis contains mL quantity of thawed, heparinized pig's blood diluted into 10mLs tap water, converted into mmol/L estimates of [Hgb]. y-axis is the formula estimate of hemoglobinuria in mmol/L.
![Spectrometer estimate of hemoglobinuria regressed against manual Hgb estimate from assumed hematocrit](img/agreementlinreg.png)


x-axis contains mL quantity of thawed, heparinized pig's blood diluted into 10mLs tap water. y-axis is the formula estimate of hemoglobinuria in mmol/L.
![Spectrometer estimate of hemoglobinuria against volume/volume concentration of pig's blood:](img/mLcorrespondence.png)


The last graph is a perfect line, as the y-axis is a manual estimate of hemoglobinuria using an assumed 7-mmol/L concentration of heme units in blood (four times the HgB concentration, as there are four hemes to a HgB). The x-axis is the mLs of blood diluted by 10mL tap water, a volume/volume dilution. 
![Manual estimate of hemoglobinuria against volume/volume concentration of pig's blood:](img/molartomL.png)


While there is exceptional linear agreement between the method proposed from spectrophotometric analysis and the manual estimate of hemoglobin concentration, the slopes of the individual estimates are different. 


### Limitations

#### Use of a single polychromatic white-LED light source.

A potential limitation of the use of LEDs in spectrophotometry is in the relatively broad spectral bandwidth of light emitted. It is often stated that obedience to Beer's law only strictly applies when measurements are made with monochromatic source radiation. In practise, the light emitted from many spectrophotometers is not truly monochromatic but a narrow range of wavelengths [12, 13]; thus, the traditional, functional distinction between colourimetry and spectrophotometry is blurred. The narrower the spectral bandwidth of a light source, the greater the ability of the spectrophotometer to distinguish between molecules present with similar absorbance spectra, and so the resolution also increases, allowing for an accurate determination of the molar absorptivity constant [12, 14]. Narrow spectral bandwidth is obviously vital for some applications but not all. Indirect methods, in which a signal molecule is formed that has a high molar absorptivity and/or is generated in high concentration, are less likely to be affected by relatively wide spectral bandwidths than direct spectrophotometric analysis of complex solutions containing molecules with similar absorbance spectra at similar concentrations/molar absorptivities. [citation](https://journals.plos.org/plosbiology/article?id=10.1371/journal.pbio.3000321#sec004)

Trvially, an alternative embodiment of the device can be constructed using LEDs made to emit at 500nm and 570nm more specifically. 
