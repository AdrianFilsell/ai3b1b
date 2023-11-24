summary
=======

convolutional neural network machine learning/inference suite written in c++.

build/confiure convolutional neural network.
build/confiure network training set.

train network using training set with cost/image feedback.
control training - play/pause/step one epoch.

network machine learning inference image export at any resolution.
network machine learning inference image blending.
network machine learning inference image morphing.

save/load network configuration and training to/from file.
if you load 'image id -> b8g8r8' data but current input layer type in main dlg is not 'image id -> b8g8r8' you will not see the data loaded, so load then select data type you wish to view/mutate.

build/configure
===============

input layer:
	user - generic type, user has to configure input layer perceptrons and supply training set item i/o values.
	       network input will be user supplied values and network output meaning is user dependent.

	image id -> b8g8r8 - app will configure input layer perceptrons and supply training set item i/o values when user adds an image to the training set.
			     network input will be an app supplied id and network output will be all pixel channel data.
			     lerp between two training set image ids and network will infer an output, this will be a blend between the two images.

	image id,x,y -> b8g8r8 - app will configure input layer perceptrons and supply training set item i/o values when user adds an image to the training set.
				 network input will be an app supplied id + normalised pixel position and network output will be the pixel channel data at that position.
				 lerp between two training set image ids and network will infer an output, this will be a morph between the two images.

	images used to create training set data will be scaled to the max dim supplied by user I typically use a max dim of 60 pixels.

hidden/output layer:
	add/delete/reposition layers
	
	configure layer properties:
		perceptrons
		activation function
		activation normalisation
		gradient clipping

training:
	weight/bias initialisation:
		auto He/Xavier - relu and relu(like) will use He initialisation, sigmoid and sigmoid(like) will use Xavier initialisation
		auto rand - relu and relu(like) will use random numbers in range [0.1,0.5], sigmoid and sigmoid(like) will use random numbers in range [-0.5,0.5]
		user rand -  random numbers in user supplied range
	learning rate - this is mutable when training in progress
	epochs - this is mutable when training in progress

control panel
=============

train/update weight/bias values in network by processing epochs:
	play - process epochs till limit reached

	pause - halt processing epochs this will allow user to use current network state for image export

	step - process one epoch if epoch limit has not yet been reached

	trash - erase all trainging done i.e. go back to state of zero epochs processed

cost panel
==========

history:
	network output layer cost graph.
update:
	configurable update interval.
	button to update now.

image panel
===========

update:
	configurable update interval.
	button to update now.

input:
	select training set image to output in thumb window ( if thumb src is input )

lerp:
	select training set image, interpolation will be between this and input image, network inferred image will be output in thumb window ( if thumb src is lerp ).
        lerp value [0,1] slider.
        animate, output all lerp values ( if thumb src is lerp ).

thumb:
	output input/output/lerp image.
	output at actual resolution or zoom in.
	smooth output using machine learning inferrence.
	export output to a file ( output image dim dlg output when appropriate )

