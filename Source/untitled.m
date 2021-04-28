[z, p, k] = butter(2, [0.1, 0.4] / 500, 'bandpass');
[sos, g] = zp2sos(double(z), double(p), double(k));
sos(end, 1 : 3) = sos(end, 1 : 3) * g;
sos(:, 5 : end) = -sos(:, 5 : end);
s = sprintf('%.60f\n', sos')
% f_40_iir_butter = sosfilt(sos, data_40);