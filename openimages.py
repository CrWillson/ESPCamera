import numpy as np
import matplotlib.pyplot as plt
from matplotlib.image import imread
import cv2

# Calculate centroid and line through a contour
def processContour(contour):
    # Calculate the centroid of the contour
    M = cv2.moments(contour)
    if M["m00"] != 0:
        cx = int(M["m10"] / M["m00"])
        cy = int(M["m01"] / M["m00"])
    else:
        cx, cy = 0, 0  # Default to (0, 0) if the contour area is zero

    # Fit a line to the contour
    [vx, vy, x0, y0] = cv2.fitLine(contour, cv2.DIST_L2, 0, 0.01, 0.01)

    # Calculate the slope from the direction vector
    if vx != 0:
        slope = vy / vx
    else:
        slope = np.inf  # Infinite slope if vx is zero (vertical line)

    # Define the line endpoints for drawing
    line_length = 100  # Extend the line beyond the image for visibility
    if slope != np.inf:
        # Calculate points along the line using the slope and centroid
        x1 = int(cx - line_length)
        y1 = int(cy - slope[0] * line_length)
        x2 = int(cx + line_length)
        y2 = int(cy + slope[0] * line_length)
    else:
        # For a vertical line, keep x constant and vary y
        x1, y1 = cx, 0
        x2, y2 = cx, 96

    #x1, y1 = max(0, min(x1, 95)), max(0, min(y1, 95))
    #x2, y2 = max(0, min(x2, 95)), max(0, min(y2, 95))

    return cx, cy, x1, y1, x2, y2

# Convert RGB565 to RGB888
def rgb565_to_rgb888(image_rgb565):
    r = ((image_rgb565[:, :, 0] & 0xF8)) >> 3
    g = ((image_rgb565[:, :, 0] & 0x07) << 3) | ((image_rgb565[:, :, 1] & 0xE0) >> 5)
    b = (image_rgb565[:, :, 1] & 0x1F)

    r = r / 32 * 255
    g = g / 64 * 255
    b = b / 32 * 255

    r = r.astype(np.uint16)
    g = g.astype(np.uint16)
    b = b.astype(np.uint16)

    return np.stack((r, g, b), axis=-1)

def open_image(file_path):
    # Load the RGB565 binary image data
    with open(file_path, 'rb') as file:
        image_data_color = file.read()

    # Convert the binary data to a numpy array and reshape it to 96x96x2
    try:
        return np.frombuffer(image_data_color, dtype=np.uint8).reshape((96, 96, 2))
    except:
        return np.frombuffer(image_data_color, dtype=np.uint8).reshape((96, 96, 1))

folder = "images/"
num_images = 39
file_names = [f"{folder}IMAGE{i+1}.BIN" for i in range(num_images)]

# Open the images
imagesRaw = [rgb565_to_rgb888(open_image(file)) for file in file_names]

# Find red areas and mask
def drawStopBox(img, mask):
    STOPBOX_TL = (45,75)
    STOPBOX_BR = (70,90)
    PERCENTTOSTOP = 10

    stopBoxMask = mask[STOPBOX_TL[1]:STOPBOX_BR[1], STOPBOX_TL[0]:STOPBOX_BR[0]]
    total_pixels = stopBoxMask.size
    red_pixels = np.count_nonzero(stopBoxMask)
    percent_red = (red_pixels / total_pixels) * 100
    if (percent_red >= PERCENTTOSTOP):
        cv2.rectangle(img, STOPBOX_TL, STOPBOX_BR, (0,255,0), 1)
    else:
        cv2.rectangle(img, STOPBOX_TL, STOPBOX_BR, (255,0,0), 1)

def processRedImg(img):
    mask = (img[:,:,0] > img[:,:,1] + 20) & (img[:,:,0] > img[:,:,2] + 30)
    redImg = np.zeros_like(img)
    redImg[mask] = img[mask]
    drawStopBox(redImg, mask)

    return redImg

redMaskedImgs = [processRedImg(img) for img in imagesRaw]


def applyWhiteCrop(img):
    whiteImg = np.copy(img)

    for x in range(96):
        for y in range(96):
            if x <= 55:
                whiteImg[x,y] = [0,0,0]
    return whiteImg

def applyWhiteMask(img):
    mask = (img[:, :, 0] > 160) & (img[:, :, 1] > 170) & (img[:, :, 2] > 100)
    mask = mask.astype(np.uint8) * 255
    whiteImg = np.zeros_like(img)
    whiteImg[mask > 0] = img[mask > 0]
    return whiteImg, mask

def applyWhiteContour(img, mask):
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours: return img

    contour = max(contours, key=cv2.contourArea)
    cx, cy, x1, y1, x2, y2 = processContour(contour)

    #cv2.line(img, (55,55), (96,96), (255,0,0), 1)
    cv2.line(img, (0,55), (96,55), (255,0,0), 1)
    cv2.drawContours(img, contour, -1, (225,255,0), 1)
    cv2.circle(img, (cx, cy), 3, (255, 0, 0), -1)
    cv2.line(img, (x1, y1), (x2, y2), (0, 255, 0), 1)

    return img

def processWhiteImg(img):
    whiteImg = applyWhiteCrop(img)
    whiteImg, mask = applyWhiteMask(whiteImg)
    whiteImg = applyWhiteContour(whiteImg, mask)
    return whiteImg

whiteMaskedImgs = [processWhiteImg(img) for img in imagesRaw]

# Display the images
for i, image in enumerate(imagesRaw):
    fig,ax = plt.subplots(2, 2, figsize=(5,5))
    fig.canvas.manager.set_window_title(f"Image {i}")
    
    ax[0,0].imshow(image)
    ax[0,0].set_title('raw image')
    ax[0,0].axis('off') 

    ax[0,1].imshow(redMaskedImgs[i])
    ax[0,1].set_title('red mask')
    ax[0,1].axis('off') 

    ax[1,0].imshow(whiteMaskedImgs[i])
    ax[1,0].set_title('white mask')
    ax[1,0].axis('off') 

    ax[1,1].imshow(image)
    ax[1,1].set_title('raw image')
    ax[1,1].axis('off') 

    plt.tight_layout()

plt.show()

print("done")
