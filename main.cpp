/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Derek Watson

- All project requirements fully met? (YES or NO):
    NO

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    <ANSWER>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


//
// YOUR FUNCTION DEFINITIONS HERE
//

const string PROCESSES [10] = {
    "Vignette", 
    "Clarendon", 
    "Grayscale", 
    "Rotate 90 degrees", 
    "Rotate multiple 90 degrees", 
    "Enlarge", 
    "High contrast", 
    "Lighten", 
    "Darken", 
    "Black, white, red, green, blue"
};

vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image) {
    return image;
}

vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor) {
    return image;
}

vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image) {
    return image;
}

vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image) {
    return image;
}

vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number) {
    return image;
}

vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale) {
    return image;
}

vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image) {
    return image;
}

vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor) {
    return image;
}

vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor) {
    return image;
}

vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image) {
    return image;
}

/**
 * Print user menu for image processing application
 * @param filename Current BMP file to work on
 * @return None
 */
void print_menu(string in_filename) {
    int menu_num = 0;
    cout << "IMAGE PROCESSING MENU" << endl
        << menu_num << ") Change image (current: " << in_filename << ")" << endl;

    for (const string &PROCESS : PROCESSES) {
        menu_num++;
        cout << menu_num << ") " << PROCESS << endl;
    }
    
    cout << endl << "Enter menu selection (Q to quit): ";
}


/**
 * Print user menu for image processing application
 * @param filename Current BMP file to work on
 * @return None
 */
void execute_selection(string* in_filename, int selection) {
    string out_filename;

    if (selection == 0) {
        cout << "Change image selected" << endl;
        cout << "Enter new input BMP filename" << endl;
        cin >> *in_filename;
        cout << "Sucessfully changed input image!" << endl;
        return;
    } else {
        cout << PROCESSES[selection] << " selected" << endl
             << "Enter output BMP filename: ";
        cin >> out_filename;
        vector<vector<Pixel>> original = read_image(*in_filename);
        vector<vector<Pixel>> modified;

        double scaling_factor;
        switch(selection) {
            case 1:
                modified = process_1(original);
                break;
            case 2:
                cout << "Enter scaling factor: ";
                cin >> scaling_factor;
                modified = process_2(original, scaling_factor);
                break;
            case 3:
                modified = process_3(original);
                break;
            case 4:
                modified = process_4(original);
                break;
            case 5:
                int rotations;
                cout << "Enter number of 90 degree rotations: ";
                cin >> rotations;
                modified = process_5(original, rotations);
                break;
            case 6:
                int x_scale, y_scale;
                cout << "Enter X scale: ";
                cin >> x_scale;
                cout << "Enter Y scale: ";
                cin >> y_scale;
                modified = process_6(original, x_scale, y_scale);
                break;
            case 7:
                modified = process_7(original);
                break;
            case 8:
                cout << "Enter scaling factor: ";
                cin >> scaling_factor;
                modified = process_8(original, scaling_factor);
                break;
            case 9:
                cout << "Enter scaling factor: ";
                cin >> scaling_factor;
                modified = process_9(original, scaling_factor);
                break;
            case 10:
                modified = process_10(original);
                break;   
        }

        write_image(out_filename, modified);
        cout << "Successfully applied " << "\"" << PROCESSES[selection] << "\"" << " effect to image!" << endl << endl;
        return;
    }
}

int main()
{
    cout << "CSPB 1300 Image Processing Application" << endl;
    
    string in_filename;
    cout << "Enter input BMP filename: ";
    cin >> in_filename;
    cout << endl;


    print_menu(in_filename);
    int selection;
    bool done = false;
    while (!done)
    {
        cin >> selection;
        if (cin.fail())
        {
            cout << "Thank you for using my program!" << endl
                 << "Quitting..." << endl << endl;
            done = true;
        }
        else
        {
            execute_selection(&in_filename, selection);
            print_menu(in_filename);
        }
    }

    return 0;
}