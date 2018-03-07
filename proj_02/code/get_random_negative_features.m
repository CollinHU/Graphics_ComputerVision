% Starter code prepared by James Hays for CS 143, Brown University
% This function should return negative training examples (non-faces) from
% any images in 'non_face_scn_path'. Images should be converted to
% grayscale, because the positive training data is only available in
% grayscale. For best performance, you should sample random negative
% examples at multiple scales.

function features_neg = get_random_negative_features(non_face_scn_path, feature_params, num_samples)
% 'non_face_scn_path' is a string. This directory contains many images
%   which have no faces in them.
% 'feature_params' is a struct, with fields
%   feature_params.template_size (probably 36), the number of pixels
%      spanned by each train / test template and
%   feature_params.hog_cell_size (default 6), the number of pixels in each
%      HoG cell. template size should be evenly divisible by hog_cell_size.
%      Smaller HoG cell sizes tend to work better, but they make things
%      slower because the feature dimensionality increases and more
%      importantly the step size of the classifier decreases at test time.
% 'num_samples' is the number of random negatives to be mined, it's not
%   important for the function to find exactly 'num_samples' non-face
%   features, e.g. you might try to sample some number from each image, but
%   some images might be too small to find enough.

% 'features_neg' is N by D matrix where N is the number of non-faces and D
% is the template dimensionality, which would be
%   (feature_params.template_size / feature_params.hog_cell_size)^2 * 31
% if you're using the default vl_hog parameters

% Useful functions:
% vl_hog, HOG = VL_HOG(IM, CELLSIZE)
%  http://www.vlfeat.org/matlab/vl_hog.html  (API)
%  http://www.vlfeat.org/overview/hog.html   (Tutorial)
% rgb2gray

org_image_files = dir( fullfile( non_face_scn_path, '*.jpg' ));
num_org_images = length(org_image_files);
n = ceil( num_samples / num_org_images);% 37
odd_num = n * num_org_images - num_samples;%138
tmp_s = feature_params.template_size;
crop = zeros(tmp_s, tmp_s, num_samples);

% get random subimages
acc = 0;
for k=1:num_org_images
    filename = fullfile(non_face_scn_path,org_image_files(k).name);  
    data = imread(filename); 
    size_d = size(data);
    imgx = size_d(1);
    imgy = size_d(2);
    if k <= odd_num
        k_size = n - 1;      
    else
        k_size = n;
    end
    for k0 = 1:k_size
        randx = randi(imgx-tmp_s, 1);
        randy = randi(imgy-tmp_s, 1);
        tmp = rgb2gray(imcrop(data,[randy randx tmp_s-1 tmp_s-1]));
        tmpidx = acc + k0;
        %tmpidx,acc, k0
        crop(:, :, tmpidx) = tmp;
    end
    acc = acc + k_size;
end 
%
features_neg = zeros(num_samples, (feature_params.template_size / feature_params.hog_cell_size)^2 * 31);
for k1=1:num_samples
    %tmpidx
    hog = vl_hog(single(crop(:,:,k1)), feature_params.hog_cell_size, 'verbose');
    features_neg(k1, :) = reshape(hog, [], 1);
end 

% placeholder to be deleted
%features_neg = rand(100, (feature_params.template_size / feature_params.hog_cell_size)^2 * 31);