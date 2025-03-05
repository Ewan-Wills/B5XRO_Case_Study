import cv2
import numpy as np

def detect_finger_circle(frame):
    # Define a Region of Interest (ROI) to focus on a specific area
    height, width = frame.shape[:2]
    roi_x, roi_y, roi_w, roi_h = width // 4, height // 4, width // 2, height // 2  # Center region
    roi = frame[roi_y:roi_y+roi_h, roi_x:roi_x+roi_w]
    
    # Convert the ROI to grayscale
    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
    
    # Apply GaussianBlur to reduce noise
    blurred = cv2.GaussianBlur(gray, (9, 9), 2)
    
    # Use Canny edge detection
    edges = cv2.Canny(blurred, 50, 150)
    
    # Find contours in the edges image
    contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    # Loop over the contours
    for contour in contours:
        # Approximate the contour to a polygon
        epsilon = 0.01 * cv2.arcLength(contour, True)
        approx = cv2.approxPolyDP(contour, epsilon, True)
        
        # Check if the contour is roughly circular
        if len(approx) > 8:  # A circle-like shape will have many vertices
            # Fit a minimum enclosing circle to the contour
            ((x, y), radius) = cv2.minEnclosingCircle(contour)
            center = (int(x) + roi_x, int(y) + roi_y)  # Adjust for ROI offset
            radius = int(radius)
            
            # Filter based on radius (adjust these values for your use case)
            min_radius = 20  # Minimum radius of the finger circle
            max_radius = 100 # Maximum radius of the finger circle
            
            if min_radius <= radius <= max_radius:
                # Draw the circle and its center on the frame
                cv2.circle(frame, center, radius, (0, 255, 0), 2)  # Draw the circle
                cv2.circle(frame, center, 2, (0, 0, 255), 3)       # Draw the center point
                
                # Display the center coordinates as numeric values
                coord_text = f"({center[0]}, {center[1]})"
                cv2.putText(frame, coord_text, (center[0] + 20, center[1] + 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
                
                return frame, center
    
    # If no circle-like contour is detected, return the original frame and None for the center
    return frame, None

def main():
    # Open the webcam
    cap = cv2.VideoCapture(0)
    
    if not cap.isOpened():
        print("Error: Could not open webcam.")
        return
    
    while True:
        # Capture frame-by-frame
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame.")
            break
        
        # Detect the finger circle and get its center
        output_frame, center = detect_finger_circle(frame)
        
        if center is not None:
            print(f"Detected circle center: {center}")
        
        # Display the resulting frame
        cv2.imshow('Finger Circle Detection', output_frame)
        
        # Break the loop if 'q' is pressed
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    # Release the capture and close windows
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()