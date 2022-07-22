# XEE Windturbine

This is an abstraction of  wind turbine. It is assembled from several static meshes for foundation, entry, tower, house, rotorhub and rotorblade. The model is controlled by windspeed and winddirection.

## Specific models

This repository contains implementations of the Nordex N117 Gamma and N117 Delta turbines.

## Implement a specific model

First of all UStaticMeshes for foundation, entry, tower, house, rotorhub and rotorblade have to be defined. These should be mapped with the correct material and textures. Additionally it is wise to implement LODs and attach them to the meshes if they are complex in highest resolution. This potentially saves optimization time later.

Select a EWindshearGroundType for correct wind shear calculations. Default is WSGT_Agricultural1KM.

Define a UCurve that describes which RPM the rotor should turn (y-axis) at which windspeed (x-axis).

Define a StartUp- and Shutdown UCurve. They describe which RPM-factor (y-axis) at which time (y-axis) the rotor should turn. These curves have a minimum of 0 and a maximum of 1. Obviously the startup curve goes from 0 to 1 in a certain time and the shutdown curve goes from 1 to 0 over a certain time.

PitchSpeed and AzimuthSpeed define the rotational speed of blade pitch and house azimuth in degrees per second.

AzimuthWinddirectionTolerance describes which deviation from winddirection the turbine acccepts before correcting the azimuth.

AngleOfAttack is related to blade pitch. The blades orient themselves in the direction of the effective airstream plus AoA.

bAWLEnabled defines if the aerial warning lights are on or off.

AWLPositions defines where Pointlights for AWl should be placed in relation to the house.

EntryRotation lets you spin the entrymesh around the tower.

NumWings defines how many rotorblades there are.

HouseHeight defines the height where the house is positioned.

Huboffset defines where the rotorhub is in relation to the house.

HubTilt defines a backwards tilted rotor axis in degrees.

## Funding
Development of this code was partially funded by the German Federal Ministry of Education and Research (BMBF) under funding reference number 13FH1I01IA.