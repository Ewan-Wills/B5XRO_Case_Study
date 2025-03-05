import cv2
import numpy as np
import screeninfo  # For detecting screen resolution

def get_screen_resolution():
    """Gets the primary screen resolution."""
    try:
        screen = screeninfo.get_monitors()[0]
        return screen.width, screen.height
    except Exception as e:
        print(f"Error getting screen resolution: {e}")
        return 1920, 1080  # Fallback resolution

def resize_image(image):
    """Resizes image to fit screen while maintaining aspect ratio."""
    try:
        screen_width, screen_height = get_screen_resolution()
    except Exception as e:
        print(f"Error getting screen resolution: {e}")
        screen_width, screen_height = 1920, 1080
        
    height, width = image.shape[:2]

    # Scale to fit within screen bounds (leaving some margin)
    max_width = int(screen_width * 0.8)
    max_height = int(screen_height * 0.8)

    scaling_factor = min(max_width / width, max_height / height)
    new_size = (int(width * scaling_factor), int(height * scaling_factor))

    return cv2.resize(image, new_size, interpolation=cv2.INTER_AREA)

def find_corners(gray):
    """Finds the corners of the wooden board using Harris Corner Detection."""
    gray = np.float32(gray)
    dst = cv2.cornerHarris(gray, blockSize=2, ksize=3, k=0.04)
    dst = cv2.dilate(dst, None)
    ret, dst = cv2.threshold(dst, 0.01 * dst.max(), 255, 0)
    dst = np.uint8(dst)
    
    # Find centroids
    ret, labels, stats, centroids = cv2.connectedComponentsWithStats(dst)
    
    # Sort by area to get the largest 4 points
    corners = sorted(centroids, key=lambda c: stats[labels[int(c[1]), int(c[0])]][4], reverse=True)[:4]
    return np.array(corners, dtype=np.int32)

def preprocess_image(image_path, output_path="processed_base.jpg", debug=True):
    """Processes the image to detect and classify screw holes."""
    try:
        img = cv2.imread(image_path)
        if img is None:
            raise FileNotFoundError(f"Image not found at {image_path}")
        
        img = resize_image(img)  # Resize to fit screen 
        original = img.copy()
        
        # Convert to grayscale
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        
        # Create debug directory
        if debug:
            debug_imgs = {}
            debug_imgs["original"] = original.copy()
            debug_imgs["gray"] = gray.copy()
        
        # Apply light blurring to preserve details
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)
        if debug:
            debug_imgs["blurred"] = blurred.copy()
        
        # Apply moderate denoising
        denoised = cv2.fastNlMeansDenoising(blurred, None, 10, 7, 21)
        if debug:
            debug_imgs["denoised"] = denoised.copy()
        
        # Enhance contrast
        clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8, 8))
        enhanced = clahe.apply(denoised)
        if debug:
            debug_imgs["enhanced"] = enhanced.copy()
         
        # Step 1: Find the main circular disk
        main_circle = identify_main_circle(enhanced)
        
        if main_circle is None:
            print("Warning: Main circular disk not detected.")
            # Use image center and approximate size as fallback
            h, w = enhanced.shape
            main_circle = (w//2, h//2, min(w, h)//3)
         
        # Create a mask for the circular region
        disk_mask = np.zeros_like(enhanced)
        cv2.circle(disk_mask, (main_circle[0], main_circle[1]), main_circle[2], 255, -1)
        if debug:
            debug_imgs["disk_mask"] = disk_mask.copy()
        
        # Apply adaptive thresholding to better handle varying lighting
        adaptive_binary = cv2.adaptiveThreshold(
            enhanced, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
            cv2.THRESH_BINARY_INV, 11, 2
        )
        if debug:
            debug_imgs["adaptive_binary"] = adaptive_binary.copy()
        
        # Apply Otsu's thresholding
        _, otsu_binary = cv2.threshold(enhanced, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)
        if debug:
            debug_imgs["otsu_binary"] = otsu_binary.copy()
        
        # Combine adaptive and Otsu's thresholding
        combined_binary = cv2.bitwise_or(adaptive_binary, otsu_binary)
        masked_binary = cv2.bitwise_and(combined_binary, combined_binary, mask=disk_mask)
        if debug:
            debug_imgs["masked_binary"] = masked_binary.copy()
        
        # Clean up binary image with light morphology
        kernel = np.ones((3, 3), np.uint8)
        opening = cv2.morphologyEx(masked_binary, cv2.MORPH_OPEN, kernel)
        closing = cv2.morphologyEx(opening, cv2.MORPH_CLOSE, kernel)
        if debug:
            debug_imgs["morphology"] = closing.copy()
        
        # Edge detection with parameters tuned for screw holes
        edges = cv2.Canny(enhanced, 30, 100)
        masked_edges = cv2.bitwise_and(edges, edges, mask=disk_mask)
        if debug:
            debug_imgs["edges"] = masked_edges.copy()
            cv2.imwrite("debug_edges.jpg", masked_edges)
        
        # Use both gradient-based and template-based circle detection
        circles1 = detect_circles_hough(enhanced, masked_edges, closing)
        
        # Add template matching as a backup method
        template_circles = detect_circles_template(gray, disk_mask)
        
        # Combine the results
        combined_circles = combine_circles(circles1, template_circles)
        
        # Detect circles using contour approach as another method
        contour_circles = detect_circles_contour(closing)
        
        # Merge all detection methods
        final_circles = merge_all_circles([combined_circles, contour_circles], main_circle)
        
        # Process and classify each detected hole
        if final_circles is not None and len(final_circles) > 0:
            # Create report
            report = []
            
            for i, circle in enumerate(final_circles):
                x, y, r = circle
                
                # Create a mask for this circle
                circle_mask = np.zeros_like(gray)
                cv2.circle(circle_mask, (x, y), r, 255, -1)
                
                # Calculate statistics within the circle
                roi = cv2.bitwise_and(gray, gray, mask=circle_mask)
                non_zero = cv2.countNonZero(circle_mask)  # Corrected here
                if non_zero > 0:
                    mean_intensity = np.sum(roi) / non_zero
                else:
                    mean_intensity = 0
                
                # Calculate standard deviation for better classification
                mean_val = cv2.mean(gray, mask=circle_mask)[0]
                sqr_diff = cv2.bitwise_and((gray - mean_val) ** 2, (gray - mean_val) ** 2, mask=circle_mask)
                std_dev = np.sqrt(np.sum(sqr_diff) / (non_zero if non_zero > 0 else 1))
                
                # Get the darkest region value
                min_val = np.min(roi[roi > 0]) if np.countNonZero(roi) > 0 else 255  # Corrected here
                
                # Check if the hole is filled or empty using multiple criteria
                if mean_intensity < 120 or min_val < 50 or std_dev > 40:
                    status = "Filled"
                    color = (0, 0, 255)  # Red for filled holes
                else:
                    status = "Empty"
                    color = (0, 255, 0)  # Green for empty holes
                
                # Draw circle and label
                cv2.circle(img, (x, y), r, color, 2)
                cv2.circle(img, (x, y), 2, color, -1)  # Center point
                
                # Add label with ID and status
                label = f"#{i+1}: {status}"
                cv2.putText(img, label, (x - r, y - r - 5), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)
                
                # Add to report
                report.append({
                    "id": i+1,
                    "position": (x, y),
                    "radius": r,
                    "status": status,
                    "mean_intensity": mean_intensity,
                    "std_dev": std_dev,
                    "min_val": min_val
                })
            
            # Save processed image
            cv2.imwrite(output_path, img)
            print(f"Processed image saved as {output_path}")
            
            # Generate report
            print("\nScrew Hole Report:")
            print("------------------")
            for hole in report:
                print(f"Hole #{hole['id']} at position {hole['position']}: {hole['status']}")
                print(f"  - Mean intensity: {hole['mean_intensity']:.1f}")
                print(f"  - Std deviation: {hole['std_dev']:.1f}")
                print(f"  - Min value: {hole['min_val']}")
            
            # Create a comprehensive debug visualization
            if debug:
                create_debug_visualization(debug_imgs, img, report, "detection_analysis.jpg")
            
            # Return results
            return img, report, closing
        else:
            print("No screw holes detected.")
            cv2.imwrite(output_path, img)
            return img, [], closing
    except Exception as e:
        print(f"Error processing image: {e}")
        return None, [], None

def detect_circles_hough(gray_img, edges_img, binary_img):
    """Detects screw holes using Hough Circle Transform with optimized parameters."""
    # Try multiple parameter sets for more robust detection
    all_circles = []
    
    # First attempt with moderate parameters 
    circles1 = cv2.HoughCircles(
        gray_img, 
        cv2.HOUGH_GRADIENT, 
        dp=1.2, 
        minDist=20,       # Allow closer circles
        param1=40,        # Canny edge detector threshold
        param2=22,        # Accumulator threshold (lower = more circles)
        minRadius=6,      # Min radius of screw holes
        maxRadius=15      # Max radius of screw holes
    )
    
    if circles1 is not None:
        all_circles.extend(circles1[0])
    
    # Second attempt with different parameters
    circles2 = cv2.HoughCircles(
        gray_img, 
        cv2.HOUGH_GRADIENT, 
        dp=1.5,           # Higher dp for better center detection
        minDist=15,       # Even closer circles
        param1=30,        # Lower edge threshold
        param2=18,        # Even lower accumulator threshold
        minRadius=5,      # Slightly smaller min radius
        maxRadius=18       # Slightly larger max radius
    )
    
    if circles2 is not None:
        all_circles.extend(circles2[0])
    
    # If we don't have enough circles, try with even more permissive parameters
    if len(all_circles) < 6:
        circles3 = cv2.HoughCircles(
            gray_img, 
            cv2.HOUGH_GRADIENT, 
            dp=1.0,
            minDist=10,
            param1=25,
            param2=15,
            minRadius=4,
            maxRadius=20
        )
        
        if circles3 is not None:
            all_circles.extend(circles3[0])
    
    if not all_circles:
        return None
    
    # Convert to numpy array for filtering
    all_circles = np.array([all_circles])
    
    # Filter circles with more permissive criteria
    filtered_circles = []
    circles = np.uint16(np.around(all_circles[0]))
    
    for circle in circles:
        x, y, r = circle
        
        # Check if center is within image bounds with margin
        h, w = gray_img.shape
        if x < r or y < r or x > w - r or y > h - r:
            continue
        
        # Create circle mask
        circle_mask = np.zeros_like(gray_img)
        cv2.circle(circle_mask, (x, y), r, 255, -1)
        
        # Check edge presence around perimeter
        ring_mask = np.zeros_like(gray_img)
        cv2.circle(ring_mask, (x, y), r, 255, 2)  # Draw just the perimeter
        edge_overlap = cv2.bitwise_and(edges_img, edges_img, mask=ring_mask)
        edge_pixels = cv2.countNonZero(edge_overlap)  # Corrected here
        perimeter = 2 * np.pi * r
        edge_ratio = edge_pixels / perimeter
        
        # Check dark pixels inside circle
        dark_overlap = cv2.bitwise_and(binary_img, binary_img, mask=circle_mask)
        dark_pixels = cv2.countNonZero(dark_overlap)  # Corrected here
        circle_area = np.pi * r * r
        dark_ratio = dark_pixels / circle_area
        
        # Very permissive filtering criteria to catch all potential holes
        if (edge_ratio > 0.05 or dark_ratio > 0.05):
            filtered_circles.append(circle)
    
    # Return filtered circles if any
    if filtered_circles:
        return filtered_circles
    else:
        return []

def detect_circles_template(gray_img, disk_mask):
    """Uses template matching to detect circular patterns.""" 
    # Create templates of different sizes
    templates = []
    for r in range(6, 16, 2):  # Different radii
        template = np.zeros((2*r+5, 2*r+5), dtype=np.uint8)
        cv2.circle(template, (r+2, r+2), r, 255, 1)
        templates.append((template, r))
    
    potential_circles = []
    
    # Apply the disk mask to limit search area
    masked_gray = cv2.bitwise_and(gray_img, gray_img, mask=disk_mask)
    
    for template, r in templates:
        # Template matching
        result = cv2.matchTemplate(masked_gray, template, cv2.TM_CCOEFF_NORMED)
        threshold = 0.5
        loc = np.where(result >= threshold)
        
        for pt in zip(*loc[::-1]):
            x = pt[0] + r + 2
            y = pt[1] + r + 2
            potential_circles.append((x, y, r))
    
    return potential_circles

def detect_circles_template(gray_img, disk_mask):
    """Uses template matching to detect circular patterns.""" 
    # Create templates of different sizes
    templates = []
    for r in range(6, 16, 2):  # Different radii
        template = np.zeros((2*r+5, 2*r+5), dtype=np.uint8)
        cv2.circle(template, (r+2, r+2), r, 255, 1)
        templates.append((template, r))
    
    potential_circles = []
    
    # Apply the disk mask to limit search area
    masked_gray = cv2.bitwise_and(gray_img, gray_img, mask=disk_mask)
    
    for template, r in templates:
        # Template matching
        result = cv2.matchTemplate(masked_gray, template, cv2.TM_CCOEFF_NORMED)
        threshold = 0.5
        loc = np.where(result >= threshold)
        
        for pt in zip(*loc[::-1]):
            x = pt[0] + r + 2
            y = pt[1] + r + 2
            potential_circles.append((x, y, r))
    
    return potential_circles

def detect_circles_contour(binary_img):
    """Detects potential screw holes using contour analysis."""
    # Find contours
    contours, _ = cv2.findContours(binary_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    potential_circles = []
    for contour in contours:
        # Filter by area to eliminate tiny noise
        area = cv2.contourArea(contour)
        if area < 30:  # Minimum area threshold
            continue
            
        # Check circularity
        perimeter = cv2.arcLength(contour, True)
        if perimeter == 0:
            continue
            
        circularity = 4 * np.pi * area / (perimeter * perimeter)
        
        # Only consider somewhat circular contours
        if circularity > 0.4:
            (x, y), radius = cv2.minEnclosingCircle(contour)
            center = (int(x), int(y))
            radius = int(radius)
            
            # Filter by radius to match screw hole size
            if 5 <= radius <= 15:
                potential_circles.append((center[0], center[1], radius))
    
    return potential_circles

def combine_circles(circles_list, template_circles):
    """Combines multiple lists of circles into one list, removing duplicates."""
    combined = set()
    for circles in circles_list:
        for circle in circles:
            combined.add(tuple(circle))
    return list(combined)

def merge_all_circles(circles_lists, main_circle):
    """Merges all circle detection methods and filters based on main circle."""
    combined_circles = combine_circles(circles_lists)
    
    # Filter circles based on distance from main circle center
    main_x, main_y, main_r = main_circle
    filtered_circles = []
    for circle in combined_circles:
        x, y, r = circle
        distance = np.sqrt((x - main_x)**2 + (y - main_y)**2)
        if distance < main_r:
            filtered_circles.append(circle)
    
    return filtered_circles

def create_debug_visualization(debug_imgs, final_img, report, output_path):
    """Creates a comprehensive debug visualization."""
    num_images = len(debug_imgs)
    cols = 3
    rows = (num_images + cols - 1) // cols
    width, height = final_img.shape[1], final_img.shape[0]
    debug_img = np.zeros((rows * height, cols * width, 3), dtype=np.uint8)
    
    for i, (name, img) in enumerate(debug_imgs.items()):
        row = i // cols
        col = i % cols
        debug_img[row*height:(row+1)*height, col*width:(col+1)*width] = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
        cv2.putText(debug_img, name, (col*width + 10, row*height + 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
    
    # Add final image with detections
    debug_img[0:height, (cols-1)*width:(cols)*width] = final_img
    
    # Add report text
    report_text = "\n".join([f"Hole #{hole['id']} at position {hole['position']}: {hole['status']}" for hole in report])
    report_lines = report_text.split('\n')
    y_offset = 10
    for line in report_lines:
        cv2.putText(debug_img, line, (10, y_offset), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
        y_offset += 20
    
    cv2.imwrite(output_path, debug_img)
    print(f"Debug visualization saved as {output_path}")

def identify_main_circle(gray_img):
    """Identifies the main circular region (wooden disk) in the image."""
    # Apply blur to reduce noise
    blurred = cv2.GaussianBlur(gray_img, (9, 9), 0)
    
    # Try multiple parameter sets for more robust detection
    circles = None
    param2_values = [30, 25, 20, 15]
    
    for param2 in param2_values:
        circles = cv2.HoughCircles(
            blurred, 
            cv2.HOUGH_GRADIENT, 
            dp=1.2, 
            minDist=100,
            param1=50,
            param2=param2,
            minRadius=50,   # Main circle should be much larger
            maxRadius=200
        )
        
        if circles is not None and len(circles[0]) > 0:
            # Sort by radius (largest first)
            sorted_circles = sorted(circles[0], key=lambda c: c[2], reverse=True)
            return np.uint16(sorted_circles[0])  # Return the largest circle
    
    # If no circles found, try different approach with more aggressive preprocessing
    if circles is None:
        # Try edge-based detection
        edges = cv2.Canny(blurred, 30, 100)
        kernel = np.ones((5, 5), np.uint8)
        dilated = cv2.dilate(edges, kernel, iterations=1)
        
        contours, _ = cv2.findContours(dilated, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if contours:
            largest_contour = max(contours, key=cv2.contourArea)
            (x, y), radius = cv2.minEnclosingCircle(largest_contour)
            return (int(x), int(y), int(radius))
    
    return None

# Example usage
if __name__ == "__main__":
    image_path = 'base_snapshot.jpg'  # Replace with your image path
    preprocess_image(image_path)