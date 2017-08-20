[y,Fs] = audioread('~/ir1.wav');
audioinfo('~/ir1.wav')

sound1 = resample(y,24,44);
sound(sound1, 24*Fs/44);

sound1 = resample(y,16,44);
%sound(sound1, 16*Fs/44);

sound1 = resample(y,8,44);
%sound(sound1, 8*Fs/44);
