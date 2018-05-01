function [ratio_images, height, width, index, num_images] = readImage(path_pos)
image_files = dir( fullfile(path_pos, '*.bmp') ); %Caltech Faces stored as .jpg
num_images = length(image_files);

%features_pos = zeros(num_images, (feature_params.template_size / feature_params.hog_cell_size)^2 * 31);
if num_images >= 1
    [height, width]= size(rgb2gray(imread(fullfile(path_pos, image_files(1).name))));
end
total_pix = height * width;
ratio_images = zeros(num_images, total_pix);

for i = 1: num_images
    img = rgb2gray(double(imread(fullfile(path_pos, image_files(i).name)))/255);
    ratio_images(i,:) = reshape(img,1,[]);
end
[B,I] = sort(ratio_images);
[X,Y] = sort(I);

L = floor(0.7 * num_images);
H = floor(0.9 * num_images);

K = sum(Y > L, 2);
R = sum(Y .* (Y > L), 2) ./ K;
%R = K .*(R < H);
[rank, index] = max(K .*(R < H));
%
%for i = 1: num_images
%    if(i ~= index)
%        ratio_images(i, :) = ratio_images(i, :) ./ ratio_images(index,:);
%    end
%end

end