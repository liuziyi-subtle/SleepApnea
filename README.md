### Butter

[z, p, k] = butter(2, [0.1 0.4] / 2.5);
[sos, g] = zp2sos(single(z), single(p), single(k));
sos(end, 1 : 3) = sos(end, 1 : 3) * g;
sos(:, 5 : end) = -sos(:, 5 : end);
s = sprintf('%.60f\n', sos')
