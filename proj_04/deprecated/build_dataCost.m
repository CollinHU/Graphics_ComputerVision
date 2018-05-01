function [A] = build_dataCost(V, N)
A = int32(pdist2(V, N) * 10000); 
end
