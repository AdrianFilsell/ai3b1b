Machine learning desktop app, convolutional neural network implementation, supports network creation/configuraton/training, trained image network can use inference to resample/blend/morph images. Network configuration and training progress can be saved/loaded. Written using MFC/C++/intel tbb/jpeglib, more details can be found here:

https://www.youtube.com/watch?v=coming soon...

Supplied 7 pre-trained '*.ml' files in the "./training dir", "*_id_xy.ml" files have been trained on 2 images and can morph images, "*_id.ml" files have been trained on 2 images andcan blend.
"*_id_xy.ml" files: i.e. "./training/lion_tiger_id_xy.ml", press 'load', open "./training/lion_tiger_id_xy.ml", make sure correct input type selected i.e. go to 'Layers' tab in main dialog and select input combo type 'Image id,x,y -> b8g8r8', open 'Image Panel' via 'Control Panel', select 'Output' from 'Thumb Src' combo, press 'Animate'.
"*_id.ml" files: i.e. "./training/lion_tiger_id.ml", press 'load', open "./training/lion_tiger_id.ml", make sure correct input type selected i.e. go to 'Layers' tab in main dialog and select input combo type 'Image id -> b8g8r8', open 'Image Panel' via 'Control Panel', select 'Output' from 'Thumb Src' combo, press 'Animate'.

Supplied pre-trained networks:
  spiderman mask <-> spiderman symbol, david banner <-> hulk, lion <-> tiger, ghostbusters

![Alt text](/training/training.png?raw=true "NN trained on Ghostbusters logo")
