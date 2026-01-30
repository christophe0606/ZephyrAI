# Generate image arrays to display the KWS words on the LCD
import numpy as np
from PIL import Image, ImageDraw, ImageFont,ImageChops

def generate_image(word, width=300, height=300, font_size=12,disp=False):
    # Create a blank image with white background
    img = Image.new('L', (width, height), color = 0)
    bg = Image.new("L", (width, height), 0)

    
    # Initialize ImageDraw
    d = ImageDraw.Draw(img)
    
    # Load a font
    try:
        font = ImageFont.truetype("arialbd.ttf", font_size)
    except IOError:
        font = ImageFont.load_default()
    
    # Calculate text size and position
    bbox = d.textbbox((0, 0), word, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    position = ((width - text_width) // 2, (height - text_height) // 2)
    
    # Add text to image
    d.text(position, word, fill=255, font=font)

    diff = ImageChops.difference(img, bg)
    bbox = diff.getbbox()

    if bbox:
        img = img.crop(bbox)
   
    if word == "RIGHT" and disp:
       img.show()
    
    # Convert image to numpy array and then to uint16 format
    img_array = np.array(img)
    
    
    if not np.any(img_array):
        print(f"Warning: The image for word '{word}' is completely black.")
    
    # Not RGB565.
    # Grayscale
    #img_array = ((img_array[:, :, 0].astype(np.uint16) >> 3) << 11) | ((img_array[:, :, 1].astype(np.uint16) >> 2) << 5) | (img_array[:, :, 2].astype(np.uint16) >> 3)
    
    img_array = img_array.astype(np.uint8)

    # Horizontal and vertical flip to match display orientation
    img_array = img_array[::-1, ::-1]
    
    return img_array

words = ["DOWN",
        "GO",
        "LEFT",
        "NO",
        "OFF",
        "ON",
        "RIGHT",
        "STOP",
        "UP",
        "YES"]

c_out_file = f"../kws_img.c"
h_out_file = f"../kws_img.h"
with open(c_out_file, 'w', encoding='utf-8') as cf:
    with open(h_out_file, 'w', encoding='utf-8') as hf:

        c_content = []
        c_content.append('#include "kws_img.h"\n')
        c_content.append('#include "arm_math_types.h"\n')
        cf.write(''.join(c_content))

        h_content = []
        h_content.append('#ifndef KWS_IMG_H\n')
        h_content.append('#define KWS_IMG_H\n\n')
        h_content.append('#include <stdint.h>\n\n')
        hf.write(''.join(h_content))
        all_widths = []
        all_heights = []

        def sanitize_name(s):
                return ''.join(c if (c.isalnum() or c == '_') else '_' for c in s)
        
        for word in words:
            img_array = generate_image(word,disp=False,font_size=70)
        
           
        
            def to_c_array_str(arr, var_name,var_width,var_height):
                # arr is a 2D numpy array of uint16
                h, w = arr.shape
                # Flatten row-major
                flat = arr.reshape(-1)
                # Format numbers, 12 elements per line
                lines = []
                per_line = 12
                for i in range(0, len(flat), per_line):
                    chunk = flat[i:i+per_line]
                    lines.append(', '.join(str(int(x)) for x in chunk))
                body = ',\n    '.join(lines)
                return f"__ALIGNED(16) static const uint8_t {var_name}[{var_width} * {var_height}] = {{\n    {body}\n}};\n"
        
            def to_h_array_str(var_name,var_width,var_height):
                return f"extern const uint8_t {var_name}[{var_width} * {var_height}];\n"


            # Prepare file content
            var_base = sanitize_name(word.lower())
            var_width = f"{var_base.upper()}_WIDTH"
            var_height = f"{var_base.upper()}_HEIGHT"
            var_name = f"kws_{var_base}_img"
            width = img_array.shape[1]
            height = img_array.shape[0]
            all_widths.append(width)
            all_heights.append(height)
        
            c_content = []
            c_content.append(f"// Image for word: {word}")
            c_content.append(to_c_array_str(img_array, var_name,var_width,var_height))
            c_content.append(f"// End of {word}\n\n")
        
            
            cf.write('\n'.join(c_content))

            h_content = []
            h_content.append(f"// Image for word: {word}")
            h_content.append(f"#define {var_width} {width}")
            h_content.append(f"#define {var_height} {height}\n")
            #h_content.append(to_h_array_str(var_name,var_width,var_height))
            h_content.append(f"// End of {word}\n\n")
        
            
            hf.write('\n'.join(h_content))
        
        c_content = []
        c_content.append(f'const uint32_t kws_widths[{len(words)}]={{')
        c_content.append(', '.join(str(w) for w in all_widths))
        c_content.append('};\n')
        c_content.append(f'const uint32_t kws_heights[{len(words)}]={{')
        c_content.append(', '.join(str(h) for h in all_heights))
        c_content.append('};\n')
        c_content.append(f'const uint8_t* kws_imgs[{len(words)}]={{')
        c_content.append(', '.join(f'kws_{sanitize_name(word.lower())}_img' for word in words))
        #c_content.append(', '.join(f'NULL' for word in words))
        c_content.append('};\n')
        cf.write(''.join(c_content))

        h_content = []
        h_content.append(f'extern const uint32_t kws_widths[{len(words)}];\n')
        h_content.append(f'extern const uint32_t kws_heights[{len(words)}];\n')
        h_content.append(f'extern const uint8_t* kws_imgs[{len(words)}];\n\n')
        hf.write(''.join(h_content))

        h_content = []
        h_content.append('#endif // KWS_IMG_H\n')
        hf.write('\n'.join(h_content))

       

