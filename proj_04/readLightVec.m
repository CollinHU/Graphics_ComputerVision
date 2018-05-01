function light_vec = readLightVec(fileName)
fileID = fopen(fileName,'r');
formatSpec = '%f %f %f';
light_vec = fscanf(fileID,formatSpec);
light_vec = transpose(reshape(light_vec,3, []));
fclose(fileID);
end