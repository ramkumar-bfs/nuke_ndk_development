#!/usr/bin/env python3
"""
Retrieve image data from the ImgToArray Nuke node.

The ImgToArray NDK node exposes pixel data via a hidden knob called 'pixel_data_ptr'
that contains the raw memory address of the PixelStore struct.

Usage (in Nuke Python console):
  import sys
  sys.path.insert(0, '/path/to/this/script')
  from get_image_data import get_image_array
  
  node = nuke.toNode('ImgToArray1')  # or whatever your node is named
  img_array = get_image_array(node)
  print(img_array.shape)  # (height, width, 4)
"""

import ctypes
import sys


def get_image_array(node):
    """
    Extract pixel data from an ImgToArray node and return as NumPy array.
    
    Args:
        node: Nuke node instance (the ImgToArray node)
    
    Returns:
        NumPy array of shape (height, width, 4) with dtype float64 (RGBA values)
    """
    # Get the raw pointer value from the hidden knob
    try:
        ptr_value = node['pixel_data_ptr'].getValue()
    except KeyError:
        raise RuntimeError(
            f"Node '{node.name()}' does not have 'pixel_data_ptr' knob. "
            "Is this an ImgToArray node?"
        )
    
    if not ptr_value or ptr_value == 0:
        raise RuntimeError("pixel_data_ptr is null or not initialized. "
                          "Has the node been validated?")
    
    # Cast the pointer to the PixelStore struct
    # PixelStore layout:
    #   std::vector<double> data;
    #   int width;
    #   int height;
    #   int channels;
    #   Lock lock;
    
    # On most 64-bit systems:
    #   std::vector<double>: 24 bytes (ptr, size, capacity)
    #   int width:            4 bytes
    #   int height:           4 bytes
    #   int channels:         4 bytes
    #   (Lock size varies, but we don't read it)
    
    class PixelStore(ctypes.Structure):
        pass
    
    # Define the vector's internal structure (simplified)
    class Vector(ctypes.Structure):
        _fields_ = [
            ("data", ctypes.POINTER(ctypes.c_double)),
            ("size", ctypes.c_size_t),
            ("capacity", ctypes.c_size_t),
        ]
    
    PixelStore._fields_ = [
        ("data_vector", Vector),
        ("width", ctypes.c_int),
        ("height", ctypes.c_int),
        ("channels", ctypes.c_int),
        # Lock is not read
    ]
    
    # Cast the raw pointer to our PixelStore struct
    pixel_store = ctypes.cast(int(ptr_value), ctypes.POINTER(PixelStore)).contents
    
    width = pixel_store.width
    height = pixel_store.height
    channels = pixel_store.channels
    
    print(f"Image dimensions: {width} x {height}, channels: {channels}")
    
    if width <= 0 or height <= 0 or channels <= 0:
        raise RuntimeError(
            f"Invalid dimensions from node: {width}x{height}x{channels}"
        )
    
    # Extract the pixel data array
    total_pixels = width * height * channels
    if pixel_store.data_vector.size < total_pixels:
        raise RuntimeError(
            f"Pixel buffer underrun: expected {total_pixels} elements, "
            f"but only {pixel_store.data_vector.size} available"
        )
    
    # Create a NumPy array view of the data (without copying)
    # Import numpy here to ensure we use the already-loaded instance if available
    try:
        import numpy as np
    except ImportError:
        np = sys.modules.get('numpy')
        if np is None:
            raise
    
    pixel_data = np.ctypeslib.as_array(
        pixel_store.data_vector.data,
        shape=(total_pixels,)
    )
    
    # Reshape to (height, width, channels)
    img_array = pixel_data.reshape((height, width, channels))
    
    # Make a copy to ensure data persists after function return
    # Import numpy here to avoid conflicts if it's already loaded elsewhere
    try:
        import numpy as np
    except ImportError:
        np = sys.modules.get('numpy')
        if np is None:
            raise
    
    return np.array(img_array, dtype=np.float64)


def save_image_to_file(node, filename):
    """
    Extract image from ImgToArray node and save as PNG or NPY.
    
    Args:
        node: Nuke node instance
        filename: Output path (e.g., 'output.png' or 'output.npy')
    """
    img_array = get_image_array(node)
    
    # Import numpy here to use already-loaded instance
    try:
        import numpy as np
    except ImportError:
        np = sys.modules.get('numpy')
        if np is None:
            raise
    
    if filename.endswith('.npy'):
        # Save as NumPy format (preserves float64)
        np.save(filename, img_array)
        print(f"Saved to {filename}")
    elif filename.endswith('.png'):
        # Convert to uint8 for PNG (assumes normalized 0-1 or 0-255 range)
        try:
            import cv2
        except ImportError:
            cv2 = sys.modules.get('cv2')
            if cv2 is None:
                raise
        
        img_uint8 = np.clip(img_array * 255, 0, 255).astype(np.uint8)
        # OpenCV expects BGR, but we have RGBA, so convert
        if img_array.shape[2] == 4:
            img_bgra = cv2.cvtColor(img_uint8, cv2.COLOR_RGBA2BGRA)
            cv2.imwrite(filename, img_bgra)
        else:
            cv2.imwrite(filename, img_uint8)
        print(f"Saved to {filename}")
    else:
        raise ValueError(f"Unsupported format: {filename}")


if __name__ == "__main__":
    print(__doc__)
