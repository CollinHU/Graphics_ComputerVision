% Starter code prepared by James Hays for CS 143, Brown University
% This function returns detections on all of the images in a given path.
% You will want to use non-maximum suppression on your detections or your
% performance will be poor (the evaluation counts a duplicate detection as
% wrong). The non-maximum suppression is done on a per-image basis. The
% starter code includes a call to a provided non-max suppression function.
function [bboxes, confidences, image_ids] = ...
    run_detector(test_scn_path, w, b, feature_params)
% 'test_scn_path' is a string. This directory contains images which may or
%    may not have faces in them. This function should work for the MIT+CMU
%    test set but also for any other images (e.g. class photos)
% 'w' and 'b' are the linear classifier parameters
% 'feature_params' is a struct, with fields
%   feature_params.template_size (probably 36), the number of pixels
%      spanned by each train / test template and
%   feature_params.hog_cell_size (default 6), the number of pixels in each
%      HoG cell. template size should be evenly divisible by hog_cell_size.
%      Smaller HoG cell sizes tend to work better, but they make things
%      slower because the feature dimensionality increases and more
%      importantly the step size of the classifier decreases at test time.

% 'bboxes' is Nx4. N is the number of detections. bboxes(i,:) is
%   [x_min, y_min, x_max, y_max] for detection i. 
%   Remember 'y' is dimension 1 in Matlab!
% 'confidences' is Nx1. confidences(i) is the real valued confidence of
%   detection i.
% 'image_ids' is an Nx1 cell array. image_ids{i} is the image file name
%   for detection i. (not the full path, just 'albert.jpg')

% The placeholder version of this code will return random bounding boxes in
% each test image. It will even do non-maximum suppression on the random
% bounding boxes to give you an example of how to call the function.

% Your actual code should convert each test image to HoG feature space with
% a _single_ call to vl_hog for each scale. Then step over the HoG cells,
% taking groups of cells that are the same size as your learned template,
% and classifying them. If the classification is above some confidence,
% keep the detection and then pass all the detections for an image to
% non-maximum suppression. For your initial debugging, you can operate only
% at a single scale and you can skip calling non-maximum suppression.

test_scenes = dir( fullfile( test_scn_path, '*.jpg' ));

%initialize these as empty and incrementally expand them.
bboxes = zeros(0,4);
confidences = zeros(0,1);
image_ids = cell(0,1);
num_cell_per_window_side = feature_params.template_size / feature_params.hog_cell_size;
cell_size = feature_params.hog_cell_size;
%window_width = cell_size * num_cell_per_window_side;
scales = [1,0.85,0.75,0.6,0.5,0.4,0.25,0.15,0.1,0.07];
stride = 1;

for i = 1:length(test_scenes)
  
    %fprintf('Detecting faces in %s\n', test_scenes(i).name)
    img = imread( fullfile( test_scn_path, test_scenes(i).name ));
    %img = single(img)/255;
    if(size(img,3) > 1)
        img = rgb2gray(img);
    end
    
    factor = 1.0;
    cur_confidences = zeros(0, 1);
    cur_bboxes = zeros(0, 4);
    cur_image_ids = zeros(0,1);
    while(factor > 0.07)
        tmpImg = imresize(img,factor);
        hog = vl_hog(single(tmpImg), cell_size);
    
        width  = (size(hog,2) - num_cell_per_window_side)/stride + 1;
        height = (size(hog,1) - num_cell_per_window_side)/stride + 1;
   
        detector_num = width * height;
    
        features_img = zeros(detector_num, (num_cell_per_window_side)^2 * 31);
        cur_scale_bboxes = zeros(detector_num, 4);
        %cur_scale_image_ids = zeros(detector_num,1);
        for cell_y = 1:height
            for cell_x = 1:width
                start_x =  1 + (cell_x - 1) * stride;
                start_y = 1 + (cell_y - 1) * stride;
                end_x = start_x + num_cell_per_window_side - 1;
                end_y = start_y + num_cell_per_window_side - 1;
                %size(hog)
                %width
                cur_hog = hog(start_y:end_y, start_x:end_x,:);
            
                index = (cell_y - 1) * width + cell_x;
                features_img(index, :) = reshape(cur_hog, 1, []);
            
                cur_x_min = ((start_x - 1) * cell_size + 1)/factor; 
                cur_y_min = ((start_y - 1) * cell_size + 1)/factor;
                cur_x_max = (end_x * cell_size)/factor;
                cur_y_max = (end_y * cell_size)/factor;
                cur_scale_bboxes(index, :) = [cur_x_min, cur_y_min, cur_x_max, cur_y_max];
            end
        end
        
        X = features_img;
        %X = normr(X);
        predict = X * w + b;
        positive = find(predict > 0.5);
 
        %You can delete all of this below.
        % Let's create 15 random detections per image
        %cur_x_min = rand(15,1) * size(img,2);
        %cur_y_min = rand(15,1) * size(img,1);
        %cur_bboxes = [cur_x_min, cur_y_min, cur_x_min + rand(15,1) * 50, cur_y_min + rand(15,1) * 50];
        %cur_confidences = rand(15,1) * 4 - 2; %confidences in the range [-2 2]
        cur_scale_image_ids(1:detector_num, 1) = {test_scenes(i).name};
    
        cur_scale_confidences = predict(       positive);
        cur_scale_bboxes      = cur_scale_bboxes(    positive,:);
        cur_scale_image_ids  = cur_scale_image_ids(positive,:);
        
        cur_image_ids = [cur_image_ids; cur_scale_image_ids];
        cur_bboxes = [cur_bboxes; cur_scale_bboxes];
        %size(cur_scale_confidences)
        cur_confidences = [cur_confidences; cur_scale_confidences];
        %non_max_supr_bbox can actually get somewhat slow with thousands of
        %initial detections. You could pre-filter the detections by confidence,
         %e.g. a detection with confidence -1.1 will probably never be
        %meaningful. You probably _don't_ want to threshold at 0.0, though. You
        %can get higher recall with a lower threshold. You don't need to modify
        %anything in non_max_supr_bbox, but you can.
        factor = factor * 0.9;
    end
    [is_maximum] = non_max_supr_bbox(cur_bboxes, cur_confidences, size(img));

    cur_confidences = cur_confidences(is_maximum,:);
    cur_bboxes      = cur_bboxes(     is_maximum,:);
    cur_image_ids   = cur_image_ids(  is_maximum,:);
 
    bboxes      = [bboxes;      cur_bboxes];
    confidences = [confidences; cur_confidences];
    image_ids   = [image_ids;   cur_image_ids];
        
    %factor = factor- 0.15;
    %img = imresize(img, factor);
end