function [weights] = build_up_neightbourMat(height, width)
value = ones(2*height*width-height-width, 1);
idx_i = zeros(2*height*width-height-width, 1);
idx_j = zeros(2*height*width-height-width, 1);
k = 0;
% organize the indices
for i = 1:height
    for j = 1:width
        if i < height
            k = k+1;
            idx_i(k) = i + (j-1) * height;
            idx_j(k) = idx_i(k) + 1;
        end
        if j < width
            k = k+1;
            idx_i(k) = i + (j-1) * height;
            idx_j(k) = idx_i(k) + height;
        end
    end
end
weights = sparse(idx_i, idx_j, value, height*width, height*width);
end