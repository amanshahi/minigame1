kumbhMela = '~/kumbhmela.jpg';
shyam = '~/shyam.jpg'


Image = rgb2gray(imread(kumbhMela));
findShyam   = rgb2gray(imread(shyam));

c = normxcorr2(findShyam,Image);

[y1, x1] = find(c==max(c(:)));
y1 = y1-size(findShyam,1);
x1 = x1-size(findShyam,2);
hFigure = figure;
hAxes  = axes;
imshow(Image,'Parent', hAxes);

%finding shyam in this image
imrect(hAxes, [x1+1, y1+1, size(findShyam,2), size(findShyam,1)]);