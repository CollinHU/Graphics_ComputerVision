function [A, V] = build_smoothCost(lambda, sigma)
[V,~] = icosphere(5);% should be 5
V = V(V(:,3) > 0,:);
%num_labels= size(V,1);
%A = zeros(num_labels, num_labels);
%tmp = sum(V,2);
%for i = 1 : numLabels
%    A(i,:) = int32(abs(tmp - tmp(i))*10000);
%end
A = int32(lambda * log(1 + pdist2(V,V) / (2 * sigma * sigma)) * 10000);
%for i = 1:num_labels
%    tmp = repmat(V(i,:),num_labels,1);
%    A(i,:) = int32(lambda * log(1 + sqrt(sum((V - tmp).^2, 2)) / (2 * sigma * sigma)) * 10000);
%end
end
