import cv2
import numpy as np
import screeninfo  # For detecting screen resolution

def get_screen_resolution():
    """Gets the primary screen resolution."""
    screen = screeninfo.get_monitors()[0]
    return screen.width, screen.height

def resize_image(image):
    """Resizes image to fit screen while maintaining aspect ratio."""
    screen_width, screen_height = get_screen_resolution()
    height, width = image.shape[:2]

    # Scale to fit within screen bounds (leaving some margin)
    max_width = int(screen_width * 0.8)
    max_height = int(screen_height * 0.8)

    scaling_factor = min(max_width / width, max_height / height)
    new_size = (int(width * scaling_factor), int(height * scaling_factor))

    return cv2.resize(image, new_size, interpolation=cv2.INTER_AREA)

def preprocess_image(image_path, output_path="base_snapshot.jpg"):
    """Loads, resizes, and enhances the image for better edge detection."""
    img = cv2.imread(image_path)
    if img is None:
        print("Error: Image not found.")
        return None, None

    img = resize_image(img)  # Resize to fit screen
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Apply CLAHE (Contrast Limited Adaptive Histogram Equalization)
    clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8,8))
    enhanced_gray = clahe.apply(gray)

    # Use adaptive thresholding to isolate dark markings relative to the base
    adaptive_thresh = cv2.adaptiveThreshold(
        enhanced_gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
        cv2.THRESH_BINARY_INV, 11, 2)

    # Apply Canny edge detection with fine-tuned thresholds
    edges = cv2.Canny(enhanced_gray, 30, 100)  # Adjusted for better visibility

    # Save the final processed image
    cv2.imwrite(output_path, img)
    print(f"Processed image saved as {output_path}")

    return img, edges, adaptive_thresh

def main(image_path):
    img, edges, thresholded = preprocess_image(image_path)
    if img is None:
        return

    cv2.imshow("Edges", edges)
    cv2.imshow("Adaptive Thresholding", thresholded)
    cv2.imshow("Resized Image", img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

# Run on uploaded image
main("base_snapshot.jpg")
