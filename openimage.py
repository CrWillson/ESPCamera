import numpy as np
import matplotlib.pyplot as plt
from matplotlib.image import imread
import cv2

# Load the binary file containing 96x96 grayscale image data
file_path = 'IMAGE1.BIN'  # Path to the binary image file
with open(file_path, 'rb') as file:
    image_data = file.read()

# Convert the binary data to a numpy array and reshape it to 96x96
grayscale_image = np.frombuffer(image_data, dtype=np.uint8).reshape((96, 96))




# Load the RGB565 binary image data
file_path = 'IMAGE2.BIN'  # Path to the binary image file
with open(file_path, 'rb') as file:
    image_data_color = file.read()

# Convert the binary data to a numpy array and reshape it to 96x96x2
image_array_color = np.frombuffer(image_data_color, dtype=np.uint8).reshape((96, 96, 2))

# Convert RGB565 to RGB888
def rgb565_to_rgb888(image_rgb565):
    r = ((image_rgb565[:, :, 0] & 0xF8)) >> 3
    g = ((image_rgb565[:, :, 0] & 0x07) << 3) | ((image_rgb565[:, :, 1] & 0xE0) >> 5)
    b = (image_rgb565[:, :, 1] & 0x1F)

    r = r / 32 * 255
    g = g / 64 * 255
    b = b / 32 * 255

    r = r.astype(np.uint8)
    g = g.astype(np.uint8)
    b = b.astype(np.uint8)

    return np.stack((r, g, b), axis=-1)

image_rgb888 = rgb565_to_rgb888(image_array_color)

# Create a redscale image by keeping only the red channel
mask = (image_rgb888[:, :, 0] > image_rgb888[:, :, 1] + 31) & (image_rgb888[:, :, 0] > image_rgb888[:, :, 2] + 40)
image_red = np.zeros_like(image_rgb888)  # Initialize with zeros (black)
image_red[mask] = image_rgb888[mask]

kernel = 9
mask_uint8 = mask.astype(np.uint8) * 255
smoothed_mask = cv2.GaussianBlur(mask_uint8, (kernel, kernel), sigmaX=0)
_, smoothed_mask = cv2.threshold(smoothed_mask, 128, 255, cv2.THRESH_BINARY)
image_red_sm = np.zeros_like(image_rgb888)  # Initialize with zeros (black)
image_red_sm[smoothed_mask > 0] = image_rgb888[smoothed_mask > 0]


# Convert the RGB image to HSV
hsv_image = cv2.cvtColor(image_rgb888, cv2.COLOR_RGB2HSV)

# Extract the Value channel (V)
value_channel = hsv_image[:, :, 2]

# Apply Canny edge detection on the Value channel
edges = cv2.Canny(value_channel, threshold1=70, threshold2=200)

# Create a color image to draw the edges
edges_colored = np.zeros_like(image_rgb888)

# Color the edges in white
edges_colored[edges > 0] = [255, 255, 255]

# Load a JPG image
jpg_image_path = 'image.jpg'  # Path to the JPG image file
jpg_image = imread(jpg_image_path)

# Set up the subplots
fig, ax = plt.subplots(2, 3, figsize=(10, 5))  # Create 1 row, 2 columns

# Display the JPG image
ax[0,0].imshow(jpg_image)
ax[0,0].set_title('Original Image')
ax[0,0].axis('off')  # Hide axes

# Display the color image
ax[0,1].imshow(image_rgb888)
ax[0,1].set_title('Color Image')
ax[0,1].axis('off')  # Hide axes

# Display the grayscale image
ax[0,2].imshow(grayscale_image, cmap='gray')  # Use gray colormap
ax[0,2].set_title('Grayscale Image')
ax[0,2].axis('off')  # Hide axes

# Display the color image
ax[1,0].imshow(image_red, cmap='gray')
ax[1,0].set_title('Red Filtered Image')
ax[1,0].axis('off')  # Hide axes

# Display the color image
ax[1,1].imshow(image_red_sm, cmap='gray')
ax[1,1].set_title('Red Smoothed Image')
ax[1,1].axis('off')  # Hide axes

# Display the color image
ax[1,2].imshow(edges_colored, cmap='gray')
ax[1,2].set_title('Edges Image')
ax[1,2].axis('off')  # Hide axes


# Show the plot
plt.tight_layout()
plt.show()
