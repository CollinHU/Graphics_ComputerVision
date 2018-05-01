function [sampled_img, Lo] = resampling(path_pos)

file_name = fullfile(path_pos,'/lightvec.txt');
light_vec = readLightVec(file_name);
light_vec = normr(light_vec);

[V,~] = icosphere(4);
V = V(V(:,3) >= 0,:);

nn_index = nearestneighbour(light_vec', V');
[unique_index, ~, rev_index] = unique(nn_index);
num_unique = length(unique_index);

weight = zeros(num_unique, 1);

Lo = V(unique_index,:);

image_files = dir( fullfile(path_pos, '*.bmp') );
num_images = length(image_files);

if num_images >= 1
    [height, width, depth]= size(imread(fullfile(path_pos, image_files(1).name)));
end

sampled_img = zeros(height, width, depth, num_unique);

for i = 1:num_images
	map_index = rev_index(i);
	w = Lo(map_index,:) * light_vec(i,:)';
	img = double(imread(fullfile(path_pos, image_files(i).name)));
	sampled_img(:,:,:,map_index) = sampled_img(:,:,:,map_index) + w * img;
    weight(map_index) = weight(map_index) + w;
end
for i = 1:num_unique
    sampled_img(:,:,:,i) = sampled_img(:,:,:,i) / weight(i);
end

end
