clear; close all; clc;

EllipseXYData;

k = 10;
plotFlag = 1; 

[Atilde, Btilde] = CalibrateEllipseData2D(Xmeas, Ymeas, k, plotFlag);

[X, Y] = CorrectEllipseData2D(Xmeas, Ymeas, Atilde, Btilde);

figure 
subplot(2,1,1)
plot(X, Y, 'bo');
xlabel('X-axis');
ylabel('Y-axis');
title('Calibrated Ellipse Data');
axis equal;

subplot(2,1,2)
plot(Xmeas, Ymeas, 'ro');
xlabel('X-axis');
ylabel('Y-axis');
title('Original Ellipse Data');
axis equal;