import sys
import tensorflow as tf
import numpy as np

prev[10]
future[10]

##define the neural network model. 1 layer with 1 neuron, single input value.
model = tf.keras.Sequential([tf.keras.layers.Dense(units=1, input_shape=[1]))

##define the loss function (mean squared error to determine loss value)
model.compile(optimizer='sgd', loss='mean_squared_error')

##provide  data for training
prev_position = np.array(prev, dtype = float)
fut_position = np.array(prev, dtype = float)

##Perform training over 500 iterations
model.fit(prev_position, fut_position, epoch=500)

##Predict output for input of 10.0
print(model.predict(np.array[10.0]))
