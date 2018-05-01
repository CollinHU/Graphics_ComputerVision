addpath('lib/gco_v3/matlab','lib/S2_Sampling_Suite/S2_Sampling_Toolbox',...
    'lib/nearestneighbour','lib/surfPeterKovesi');
path_pos = 'data02';


[N, height, width, dem_img] = estimate_normalMap(path_pos);
%V = ico 

neighbours_M = build_neightbourMat(height, width);

lambda = 0.5;    % weight of smoothness term
sigma = 0.65; 
[smoothCost_M, V] = build_smoothCost(lambda, sigma);

dataCost_M = int32(pdist2(V, N) * 10000); 
num_labels = size(V, 1);

h = GCO_Create(height*width,num_labels);


GCO_SetDataCost(h,dataCost_M);
GCO_SetSmoothCost(h,smoothCost_M);

GCO_SetNeighbors(h,neighbours_M);
GCO_Expansion(h); 
labels = GCO_GetLabeling(h);
normal_map = V(labels,:);
n_map = reshape(V(labels,:),height,width,3);
figure('Name','Initial Noraml'), ...
    imshow((-1/sqrt(3) * n_map(:,:,1) + 1/sqrt(3) * n_map(:,:,2) + 1/sqrt(3) * n_map(:,:,3)) / 1.1);

[E D S] = GCO_ComputeEnergy(h);
GCO_Delete(h);

recsurf = buildModel(n_map, dem_img);