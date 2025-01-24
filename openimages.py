import numpy as np
import matplotlib.pyplot as plt
from matplotlib.image import imread
import cv2
import math

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
num_images = 52
file_names = [f"{folder}IMAGE{i+1}.BIN" for i in range(num_images)]

# Open the images
imagesRaw = [rgb565_to_rgb888(open_image(file)) for file in file_names]

def applyRedMask(img):
    mask = (img[:,:,0] > img[:,:,1] + 20) & (img[:,:,0] > img[:,:,2] + 30)
    redImg = np.zeros_like(img)
    redImg[mask] = img[mask]
    return redImg, mask

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
    redImg, mask = applyRedMask(img)
    drawStopBox(redImg, mask)

    return redImg

redMaskedImgs = [processRedImg(img) for img in imagesRaw]


def applyWhiteCrop(img, cropHeight = 55):
    whiteImg = np.copy(img)

    for x in range(96):
        for y in range(96):
            if x <= cropHeight:
                whiteImg[x,y] = [0,0,0]

    cv2.line(whiteImg, (0,cropHeight), (96,cropHeight), (255,0,0), 1)
    return whiteImg

def applyWhiteMask(img):
    mask = (img[:, :, 0] > 170) & (img[:, :, 1] > 170) & (img[:, :, 2] > 100)
    mask = mask.astype(np.uint8) * 255
    whiteImg = np.zeros_like(img)
    whiteImg[mask > 0] = img[mask > 0]
    return whiteImg, mask

def applyWhiteContour(img, mask):
    newImg = np.copy(img)
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours: return newImg

    contour = max(contours, key=cv2.contourArea)
    cx, cy, x1, y1, x2, y2 = processContour(contour)

    #cv2.line(img, (55,55), (96,96), (255,0,0), 1)
    #cv2.line(newImg, (0,55), (96,55), (255,0,0), 1)
    cv2.drawContours(newImg, contour, -1, (255,30,0), 1)
    cv2.circle(newImg, (cx, cy), 3, (255, 0, 255), -1)
    cv2.line(newImg, (x1, y1), (x2, y2), (0, 255, 0), 1)

    return newImg

def processWhiteImg(img):
    whiteImg = applyWhiteCrop(img)
    whiteImg, mask = applyWhiteMask(whiteImg)
    whiteImg = applyWhiteContour(whiteImg, mask)
    return whiteImg

whiteMaskedImgs = [processWhiteImg(img) for img in imagesRaw]


def distBtwnPoints(x1: int, y1: int, x2: int, y2: int) -> int:
    dx = x2 - x1
    dy = y2 - y1
    distsqr = dx*dx + dy*dy
    return round(math.sqrt(distsqr))

def applyWhiteContourV2(img, mask):
    newImg = np.copy(img)
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours: return newImg

    # Get centroid location of the contour
    contour = max(contours, key=cv2.contourArea)
    M = cv2.moments(contour)
    c_x, c_y = 0, 0
    if M["m00"] != 0:
        c_x = int(M["m10"] / M["m00"])
        c_y = int(M["m01"] / M["m00"])
    
    # Get the top most, right most point of the contour
    topLeftMost = min(contour, key=lambda point: (point[0][1], point[0][0]))
    tl_x, tl_y = map(int, topLeftMost[0])
    
    # Get the bottom most, left most point of the contour
    botLeftMost = max(contour, key=lambda point: (point[0][1], -point[0][0]))
    bl_x, bl_y = map(int, botLeftMost[0])
    
    
    cv2.line(newImg, (c_x, 0), (c_x, 95), (255, 0, 255), 1)
    cv2.line(newImg, (tl_x, 0), (tl_x, 95), (0, 255, 0), 1)
    cv2.line(newImg, (bl_x, 0), (bl_x, 95), (0, 0, 255), 1)
    
    cv2.line(newImg, (0, c_y), (95, c_y), (255, 0, 255), 1)
    cv2.line(newImg, (0, tl_y), (95, tl_y), (0, 255, 0), 1)
    cv2.line(newImg, (0, bl_y), (95, bl_y), (0, 0, 255), 1)
    
    font = cv2.FONT_HERSHEY_SIMPLEX
    font_scale = 0.3
    colorWhite = (255, 255, 255)
    thickness = 1
    
    WHITELINE_CENTER_POS = 28
    cv2.line(newImg, (WHITELINE_CENTER_POS, 0), (WHITELINE_CENTER_POS, 95), (255, 0, 0), 1)
    
    GOAL_CONTOUR_WIDTH = 20
    CENTER_WEIGHT = 1
    WIDTH_WEIGHT = 0
    steeringValue = WIDTH_WEIGHT*((bl_x - tl_x) - GOAL_CONTOUR_WIDTH) + CENTER_WEIGHT*(tl_x - WHITELINE_CENTER_POS)
    steeringStr = f"{steeringValue:.1f}"
    
    text = str(steeringStr)
    (text_width, text_height), baseline = cv2.getTextSize(text, font, font_scale, thickness)
    x = newImg.shape[1] - text_width - 2  # Subtract 2 pixels for padding
    y = text_height + 2  # Add 2 pixels for padding
    cv2.putText(newImg, text, (x, y), font, font_scale, colorWhite, thickness)
    
    
    return newImg
    

def processWhiteImgV2(img):
    whiteImg = applyWhiteCrop(img, 45)
    whiteImg, mask = applyWhiteMask(whiteImg)
    whiteImg = applyWhiteContourV2(whiteImg, mask)
    return whiteImg

whiteMaskedImgsV2 = [processWhiteImgV2(img) for img in imagesRaw]



def drawCarBox(img, mask):
    CARBOX_TL = (0,40)
    CARBOX_BR = (15,70)
    PERCENTTOSTOP = 10

    carBoxMask = mask[CARBOX_TL[1]:CARBOX_BR[1], CARBOX_TL[0]:CARBOX_BR[0]]
    total_pixels = carBoxMask.size
    car_pixels = np.count_nonzero(carBoxMask)
    percent_car = (car_pixels / total_pixels) * 100
    if (percent_car >= PERCENTTOSTOP):
        cv2.rectangle(img, CARBOX_TL, CARBOX_BR, (0,255,0), 1)
    else:
        cv2.rectangle(img, CARBOX_TL, CARBOX_BR, (255,0,0), 1)

def processCarImg(img):
    mask = (img[:,:,1] > img[:,:,0] + 50) & (img[:,:,1] > img[:,:,2] + 30)
    carImg = np.zeros_like(img)
    carImg[mask] = img[mask]
    drawCarBox(carImg, mask)

    return carImg

carMaskedImgs = [processCarImg(img) for img in imagesRaw]




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

    ax[1,1].imshow(whiteMaskedImgsV2[i])
    ax[1,1].set_title('new white image')
    ax[1,1].axis('off') 

    plt.tight_layout()


"""
whiteLineRaw = imagesRaw[8]
croppedWhiteImg = applyWhiteCrop(whiteLineRaw)
maskedWhiteImg, whiteMask = applyWhiteMask(croppedWhiteImg)
contouredWhiteImg = applyWhiteContour(maskedWhiteImg, whiteMask)

redLineRaw = imagesRaw[19]
maskedRedImg, redMask = applyRedMask(redLineRaw)
stopBoxRedImg = np.copy(maskedRedImg)
drawStopBox(stopBoxRedImg, redMask)

fig,ax = plt.subplots(2, 2, figsize=(5,5))
fig.canvas.manager.set_window_title(f"White Line Processing Pipeline")

ax[0,0].imshow(whiteLineRaw)
ax[0,0].set_title("1. Raw image")
ax[0,0].axis('off')

ax[0,1].imshow(croppedWhiteImg)
ax[0,1].set_title("2. Apply crop")
ax[0,1].axis('off')

ax[1,0].imshow(maskedWhiteImg)
ax[1,0].set_title("3. Mask white pixels")
ax[1,0].axis('off')

ax[1,1].imshow(contouredWhiteImg)
ax[1,1].set_title("4. Find largest contour")
ax[1,1].axis('off')

fig,ax = plt.subplots(1, 3, figsize=(10,2.5))
fig.canvas.manager.set_window_title(f"Red Line Processing Pipeline")

ax[0].imshow(redLineRaw)
ax[0].set_title("1. Raw image")
ax[0].axis('off')

ax[1].imshow(maskedRedImg)
ax[1].set_title("2. Mask red pixels")
ax[1].axis('off')

ax[2].imshow(stopBoxRedImg)
ax[2].set_title("3. Check Stop Box")
ax[2].axis('off')

plt.tight_layout()
"""

plt.show()
print("done")
