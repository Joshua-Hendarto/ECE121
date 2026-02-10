function [Anoise,Hnoise,Adist,Bdist] = CreateTumbleData(npts)
% function [Anoise,Hnoise,Adist,Bdist] = CreateTumbleData(npts)
% 
% Function to create fake tumble data in the correct units so that students
% can test their algorithms and confirm that everything is working
%
% This is based on the sensor output of 16-bits, where the sensors are set
% to 2g for accel, and 16-bits on Mags. 
%
% Updated: 1/27/2024 - evetha@ucsc.edu - Fixed to BNO055 specs

DistScale = 0.2;                    % distortion scaling of sensor (in %)
NoiseScale = 0.05;                  % noise scaling (in %)
Xcouple = 1/5;                      % cross coupling distortion relative

% From BNO055 datasheet
% Specs are not used because they don't seem to output the right values
% AccRange = 8;                         % Accelerometer range assuming 8g
% AccResolution = 14;                   % Accelerometer resolution 
% MagResolution = [13, 13, 15];         % Magnetometer resolution 
% MagFieldRange = [1300, 1300, 2500];   % Magnetometer range in uT

% Hardcoded to LSB -> Value
AscaleFactor = 1000;
HscaleFactor = 1/16;

He = [22770;5329;41510.2]/1000;     % Earth's magnetic field in uT (NED)
Ge = [0;0;1];                       % Earth's gravitational field in g (NED)

Adist = eye(3) + DistScale*diag(rand(3,1)-0.5) + ... 
       DistScale*Xcouple*(rand(3,3)-0.5);       % distortion matrix
Bdist = rand(3,1)-0.5;                          % bias vector

NoiA = NoiseScale*norm(Ge)*randn(npts,3);  % noise for accelerometer
NoiH = NoiseScale*norm(He)*randn(npts,3);  % noise for magnetometer

A = zeros(npts,3);
H = zeros(npts,3);

for i = 1:npts,
    R = CreateRandomAttitude();
    A(i,:) = (R*Ge)';
    H(i,:) = (R*He)';
end

Anoise = (Adist*A')' + norm(Ge)*ones(size(A))*Bdist + NoiA;
Hnoise = (Adist*H')' + norm(He)*ones(size(A))*Bdist + NoiH;

Anoise = floor(Anoise * AscaleFactor);
Hnoise = floor(Hnoise / HscaleFactor);

% figure(gcf), clf
% sphere(50),hold on
% colormap('gray');
% plot3(He(1),He(2),He(3),'r.');
% plot3(H(:,1),H(:,2),H(:,3),'g.');
% plot3(Hnoise(:,1),Hnoise(:,2),Hnoise(:,3),'b.');
% axis('equal');

end