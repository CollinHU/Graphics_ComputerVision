function [R, height, width, denominator_img] = estimate_normalMap(path_pos)

[sampled_img, Lo] = resampling(path_pos);

num_images = size(sampled_img, 4);
height = size(sampled_img,1);
width = size(sampled_img,2);
num_pix = height * width;

grayImgs = zeros(num_images, num_pix);
for i = 1:num_images
    grayImgs(i,:) = reshape(rgb2gray(sampled_img(:,:,:,i)/255),1,[]);
end


[~,I] = sort(grayImgs);
[~,Y] = sort(I);

L = 0.7 * num_images;
H = 0.9 * num_images;

K = sum(Y > L, 2);
R = sum(Y .* (Y > L), 2) ./ K;
%R = K .*(R < H);

[~, denominator_index] = max(K .*(R < H));

denominator_img =  sampled_img(:,:,:,denominator_index)/255;
figure('Name','denominator image');
imshow(denominator_img);

light_two = Lo(denominator_index,:);
I_two = grayImgs(denominator_index, :);

N = zeros(num_pix, 3, num_images);
R = zeros(num_pix, 3);

for j = 1 : num_images
    if(j ~= denominator_index)
        light_one = Lo(j,:);
        light = [light_two; -light_one];
        I_one = grayImgs(j,:);
        Im = [I_one; I_two]';
        N(:,:, j) = Im * light;
    end
end

for i = 1 : num_pix
A = (reshape(N(i,:,:), 3, []))';
[~, ~, V] = svd(A, 0);
R(i,:) = V(:,end);

R(i,:) = sign(V(3,3)) * V(:,3); 
end

norm_R = reshape(R,height, width,3);
figure('Name','Initial Noraml'), ...
    imshow((-1/sqrt(3) * norm_R(:,:,1) + 1/sqrt(3) * norm_R(:,:,2) + 1/sqrt(3) * norm_R(:,:,3)) / 1.1);

end
