clear; close all; clc;

%magentic field accuracy (2/7)
Horizontal_Intensity = 23208.8; %nT
Vertical_Comp = 40907.7; %nT

%raw x values
xraw1 = readmatrix('xpos.csv');
xraw2 = readmatrix('xneg.csv');

%raw y values
yraw1 = readmatrix('ypos.csv');
yraw2 = readmatrix('yneg.csv');

%raw z values
zraw1 = readmatrix('zpos.csv');
zraw2 = readmatrix('zneg.csv');

%getting average of resting values
xpos = mean(xraw1(:,1));
xneg = mean(xraw2(:,1));

ypos = mean(yraw1(:,1));
yneg = mean(yraw2(:,1));

zpos = mean(zraw1(:,1));
zneg = mean(zraw2(:,1));

%finding scale values
scale_x = (xpos - xneg) / 2;
scale_y = (ypos - yneg) / 2;
scale_z = (zpos - zneg) / 2;

%finding bias value
bias_x = (xpos + xneg) / 2
bias_y = (ypos + yneg) / 2
bias_z = (zpos + zneg) / 2

%verifying for +-g
mean_xpos_cal = mean((xraw1(:,1) - bias_x) / scale_x)
mean_xneg_cal = mean((xraw2(:,1) - bias_x) / scale_x)

mean_ypos_cal = mean((yraw1(:,1) - bias_y) / scale_y)
mean_yneg_cal = mean((yraw2(:,1) - bias_y) / scale_y)

mean_zpos_cal = mean((zraw1(:,1) - bias_z) / scale_z)
mean_zneg_cal = mean((zraw2(:,1) - bias_z) / scale_z)

