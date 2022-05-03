/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Derek Watson

- All project requirements fully met? (YES or NO):
    YES

- If no, please explain what you could not get to work:
    N/A

- Did you do any optional enhancements? If so, please explain:
    No, but I did create a few functions to test my outputs. One to write all images
    to visually test the outputs and another using the "tiny" vector discussed on moodle.
    Those funtion calls are commented out in main.
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <iomanip>
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


const int PROCESS_COUNT = 10;
const string PROCESSES [PROCESS_COUNT] = {
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


/**
 * Apply vignette to image
 * @param image Vector of vector of pixels making up the image
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            double red_value = image[row][col].red;
            double green_value = image[row][col].green;
            double blue_value = image[row][col].blue;


            double distance = sqrt(pow((col - width/2), 2) + pow((row - height/2), 2));
            double scaling_factor = (height - distance) / height;


            Pixel new_pixel;
            new_pixel.red = round(red_value * scaling_factor);
            new_pixel.green = round(green_value * scaling_factor);
            new_pixel.blue = round(blue_value * scaling_factor);
            output_image[row][col] = new_pixel;
        }
    }

    return output_image;
}


/**
 * Apply clarendon to image
 * @param image Vector of vector of pixels making up the image
 * @param scaling_factor double between 0-1 represnting intensity of effect
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            double red_value = image[row][col].red;
            double green_value = image[row][col].green;
            double blue_value = image[row][col].blue;

            double average_value = (blue_value + green_value + red_value) / 3;
            
            Pixel new_pixel;
            if (average_value >= 170)
            {
                new_pixel.red = round(255 - (255 - blue_value)*scaling_factor);
                new_pixel.green = round(255 - (255 - green_value)*scaling_factor);
                new_pixel.blue =  round(255 - (255 - red_value)*scaling_factor);
            }
            else if (average_value < 90)
            {
                new_pixel.red = red_value*scaling_factor;
                new_pixel.green = green_value*scaling_factor;
                new_pixel.blue =  blue_value*scaling_factor;
            }
            else 
            {
                new_pixel.red = red_value;
                new_pixel.green = green_value;
                new_pixel.blue =  blue_value;
            }

            output_image[row][col] = new_pixel;
        }
    }

    // return output_img;
    return output_image;
}


/**
 * Apply greyscale to image
 * @param image Vector of vector of pixels making up the image
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            double red_value = image[row][col].red;
            double green_value = image[row][col].green;
            double blue_value = image[row][col].blue;


            double gray_value = (blue_value + green_value + red_value) / 3;


            Pixel new_pixel;
            new_pixel.red = gray_value;
            new_pixel.green = gray_value;
            new_pixel.blue = gray_value;
            output_image[row][col] = new_pixel;
        }
    }

    return output_image;
}


/**
 * Apply 90 degree rotation to image
 * @param image Vector of vector of pixels making up the image
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image(width, vector<Pixel>(height));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            output_image[col][(height-1) - row] = image[row][col];
        }
    }

    return output_image;
}


/**
 * Apply 90 degree rotation to image multiple times
 * @param image Vector of vector of pixels making up the image
 * @param number Number of times the image should be rotated 90 degrees
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image = image;

    while (number > 0) 
    {
        output_image = process_4(output_image);
        number--;
    }
    return output_image;
}


/**
 * Enlarge image in x and y direction
 * @param image Vector of vector of pixels making up the image
 * @param x_scale Amount longer the image should be on x axis
 * @param y_scale Amount longer the image should be on y axis
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image(height * y_scale, vector<Pixel>(width * x_scale));

    for (int row = 0; row < round(height * y_scale); row++)
    {
        for (int col = 0; col < round(width * x_scale); col++)
        {
            Pixel new_pixel;
            new_pixel.red = image[round(row/y_scale)][round(col/x_scale)].red;
            new_pixel.green = image[round(row/y_scale)][round(col/x_scale)].green;
            new_pixel.blue = image[round(row/y_scale)][round(col/x_scale)].blue;

            output_image[row][col] = new_pixel;
        }
    }

    return output_image;
}

/**
 * Convert image to high contrast (black and white only)
 * @param image Vector of vector of pixels making up the image
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            double red_value = image[row][col].red;
            double green_value = image[row][col].green;
            double blue_value = image[row][col].blue;

            double gray_value = (red_value + green_value + blue_value) / 3;

            Pixel new_pixel;
            if (gray_value >= 255/2) 
            {
                new_pixel.red = 255;
                new_pixel.green = 255;
                new_pixel.blue = 255;
            }
            else 
            {
                new_pixel.red = 0;
                new_pixel.green = 0;
                new_pixel.blue = 0;
            }


            output_image[row][col] = new_pixel;
        }
    }

    return output_image;
}


/**
 * Lightens image
 * @param image Vector of vector of pixels making up the image
 * @param scaling_factor Amount to lighten image
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            double red_value = image[row][col].red;
            double green_value = image[row][col].green;
            double blue_value = image[row][col].blue;

            Pixel new_pixel;
            new_pixel.red = round(255 - (255 - red_value) * scaling_factor);
            new_pixel.green = round(255 - (255 - green_value) * scaling_factor);
            new_pixel.blue = round(255 - (255 - blue_value) * scaling_factor);
            output_image[row][col] = new_pixel;
        }
    }

    return output_image;
}


/**
 * Darkens image
 * @param image Vector of vector of pixels making up the image
 * @param scaling_factor Amount to darken image
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            double red_value = image[row][col].red;
            double green_value = image[row][col].green;
            double blue_value = image[row][col].blue;

            Pixel new_pixel;
            new_pixel.red = round(red_value * scaling_factor);
            new_pixel.green = round(green_value * scaling_factor);
            new_pixel.blue = round(blue_value * scaling_factor);
            output_image[row][col] = new_pixel;
        }
    }

    return output_image;
}


/**
 * Converts to only black, white, red, blue and green
 * @param image Vector of vector of pixels making up the image
 * @return Vector of vector of pixels making up the image
 */
vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image) {
    double height = image.size();
    double width = image[0].size();
    vector<vector<Pixel>> output_image(height, vector<Pixel>(width));

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            double red_value = image[row][col].red;
            double green_value = image[row][col].green;
            double blue_value = image[row][col].blue;
            double max_color = max({red_value, green_value, blue_value});

            Pixel new_pixel;
            if ((red_value + green_value + blue_value) >= 550)
            {
                new_pixel.red = 255;
                new_pixel.green = 255;
                new_pixel.blue = 255;
            }
            else if ((red_value + green_value + blue_value) <= 150)
            {
                new_pixel.red = 0;
                new_pixel.green = 0;
                new_pixel.blue = 0;
            }
            else if (max_color == red_value)
            {
                new_pixel.red = 255;
                new_pixel.green = 0;
                new_pixel.blue = 0;
            }
            else if (max_color == green_value)
            {
                new_pixel.red = 0;
                new_pixel.green = 255;
                new_pixel.blue = 0;
            }
            else
            {
                new_pixel.red = 0;
                new_pixel.green = 0;
                new_pixel.blue = 255;
            }

            output_image[row][col] = new_pixel;
        }
    }

    return output_image;
}


/**
 * Print pixel values of an image in an organized format
 * @param image Vector of vector of pixels making up the image
 * @return None
 */
void print_pixel_values(const vector<vector<Pixel>>& image ) {
    // Print all pixel values in a 2D vector of pixels
    for (int row = 0; row < image.size(); row++)
    {
        for (int col = 0; col < image[0].size(); col++)
        {
            std::cout << setw(3) << image[row][col].red << " ";
            std::cout << setw(3) << image[row][col].green << " ";
            std::cout << setw(3) << image[row][col].blue << " ";
        }
        std::cout << endl;
    }
    std::cout << endl;
}


/**
 * Test each process on tiny "image", printing resulting values
 * @return None
 */
void tiny_test() {
    // Create tiny image 2d vector of pixels
    vector<vector<Pixel>> tiny =
    {
        {{  0,  5, 10},{ 15, 20, 25},{ 30, 35, 40},{ 45, 50, 55}},
        {{ 60, 65, 70},{ 75, 80, 85},{ 90, 95,100},{105,110,115}},
        {{120,125,130},{135,140,145},{150,155,160},{165,170,175}}
    };

    // Print output after applying each process
    vector<vector<Pixel>> p1 = process_1(tiny);
    std::cout << "Process 1 Values:" << endl;
    print_pixel_values(p1);

    vector<vector<Pixel>> p2 = process_2(tiny, 0.3);
    std::cout << "Process 2 Values:" << endl;
    print_pixel_values(p2);
    
    vector<vector<Pixel>> p3 = process_3(tiny);
    std::cout << "Process 3 Values:" << endl;
    print_pixel_values(p3);
    
    vector<vector<Pixel>> p4 = process_4(tiny);
    std::cout << "Process 4 Values:" << endl;
    print_pixel_values(p4);
    
    vector<vector<Pixel>> p5 = process_5(tiny, 2);
    std::cout << "Process 5 Values:" << endl;
    print_pixel_values(p5);
    
    vector<vector<Pixel>> p6 = process_6(tiny, 2, 3);
    std::cout << "Process 6 Values:" << endl;
    print_pixel_values(p6);
    
    vector<vector<Pixel>> p7 = process_7(tiny);
    std::cout << "Process 7 Values:" << endl;
    print_pixel_values(p7);
    
    vector<vector<Pixel>> p8 = process_8(tiny, 0.5);
    std::cout << "Process 8 Values:" << endl;
    print_pixel_values(p8);
    
    vector<vector<Pixel>> p9 = process_9(tiny, 0.5);
    std::cout << "Process 9 Values:" << endl;
    print_pixel_values(p9);
    
    vector<vector<Pixel>> p10 = process_10(tiny);
    std::cout << "Process 10 Values:" << endl;
    print_pixel_values(p10);
}


/**
 * Run all processes on sample.bmp to visualize results
 * @return None
 */
void write_all_proceses() {
    vector<vector<Pixel>> original;
    // Try to read sample.bmp, abort upon error
    try 
    {
        original = read_image("sample.bmp");
        if (original.empty()) 
        {
            throw -1;
        }
    }
    catch (...)
    {
        std::cout << "No sample.bmp or error with sample.bmp." << endl;
        return;
    }
    
    // Run every process on sample.bmp, outputting each result to different file
    vector<vector<Pixel>> p1 = process_1(original);
    write_image("p1.bmp", p1);

    vector<vector<Pixel>> p2 = process_2(original, 0.3);
    write_image("p2.bmp", p2);

    vector<vector<Pixel>> p3 = process_3(original);
    write_image("p3.bmp", p3);

    vector<vector<Pixel>> p4 = process_4(original);
    write_image("p4.bmp", p4);

    vector<vector<Pixel>> p5 = process_5(original, 2);
    write_image("p5.bmp", p5);

    vector<vector<Pixel>> p6 = process_6(original, 2, 3);
    write_image("p6.bmp", p6);

    vector<vector<Pixel>> p7 = process_7(original);
    write_image("p7.bmp", p7);

    vector<vector<Pixel>> p8 = process_8(original, 0.5);
    write_image("p8.bmp", p8);

    vector<vector<Pixel>> p9 = process_9(original, 0.5);
    write_image("p9.bmp", p9);

    vector<vector<Pixel>> p10 = process_10(original);
    write_image("p10.bmp", p10);

    return;
}


/**
 * Print user menu for image processing application
 * @param filename Current BMP file to work on
 * @return None
 */
void print_menu(string in_filename) {
    // Pring menu title and option 0
    int menu_num = 0;
    std::cout << endl << "IMAGE PROCESSING MENU" << endl
        << menu_num << ") Change image (current: " << in_filename << ")" << endl;

    // Loop through processes, printing menu option for each
    for (const string &PROCESS : PROCESSES) {
        menu_num++;
        std::cout << menu_num << ") " << PROCESS << endl;
    }
    
    std::cout << endl << "Enter menu selection (Q to quit): ";
}


/**
 * Convert selection from string to int, handle input errors
 * @param selection User selected menu number as string
 * @return selection as int or negative code
 */
int convert_selection(string selection) {
    int selection_int;

    // If user enters q, return -1 to indicate quitting program
    if (selection == "Q" || selection == "q")
    {
        return -1;
    }

    // Try to convert selection to int, if conversion fails, set selection as out of bounds int
    try
    {
        selection_int = stoi(selection);
    }
    catch (...)
    {
        selection_int = PROCESS_COUNT + 1;
    }

    return selection_int;
}



/**
 * Execute user selection for menu
 * @param filename Reference to current input filename
 * @param selection integer reflection user menu selection
 * @return None
 */
void execute_selection(string* in_filename, int selection) {
    string out_filename;

    // Handle change file
    if (selection == 0) 
    {
        std::cout << "Change image selected" << endl;
        std::cout << "Enter new input BMP filename: ";
        std::cin >> *in_filename;
        std::cout << endl << "Sucessfully changed input image!" << endl;
        return;
    } 
    // If selection is inbounds, attempt to process image with specified effect
    else if (selection >= 1 && selection <= PROCESS_COUNT) 
    {
        vector<vector<Pixel>> original;
        vector<vector<Pixel>> modified;
        // Try to read current input file, print error and send back to menu if fails
        try 
        {
            original = read_image(*in_filename);
            if (original.empty()) 
            {
                throw -1;
            }
        }
        catch (...) 
        {
            std::cout << endl << "Error with input image, please choose a different file and try again." << endl;
            return;
        }

        // Get output filename from user
        std::cout << PROCESSES[selection - 1] << " selected" << endl
             << "Enter output BMP filename: ";
        std::cin >> out_filename;


        // Use switch to apply correct effect based on selection
        double scaling_factor;
        switch(selection) 
        {
            case 1:
                modified = process_1(original);
                break;
            case 2:
                std::cout << "Enter scaling factor: ";
                std::cin >> scaling_factor;
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
                std::cout << "Enter number of 90 degree rotations: ";
                std::cin >> rotations;
                modified = process_5(original, rotations);
                break;
            case 6:
                int x_scale, y_scale;
                std::cout << "Enter X scale: ";
                std::cin >> x_scale;
                std::cout << "Enter Y scale: ";
                std::cin >> y_scale;
                modified = process_6(original, x_scale, y_scale);
                break;
            case 7:
                modified = process_7(original);
                break;
            case 8:
                std::cout << "Enter scaling factor: ";
                std::cin >> scaling_factor;
                modified = process_8(original, scaling_factor);
                break;
            case 9:
                std::cout << "Enter scaling factor: ";
                std::cin >> scaling_factor;
                modified = process_9(original, scaling_factor);
                break;
            case 10:
                modified = process_10(original);
                break;
        } 

        // Attempt to write image based on outputted 2D vector, notify user of success or failure
        try 
        {
            bool success = write_image(out_filename, modified);
            if (!success) 
            {
                throw -1;
            }
        }
        catch (...) 
        {
            std::cout << endl << "Error processing image, check output file and try again." << endl;
            return;
        }
        std::cout << "Successfully applied " << "\"" << PROCESSES[selection - 1] << "\"" << " effect to image!" << endl << endl;
        return;    
    }
    // If entry is out of bounds, notify user and kick back to menu
    else 
    {
        std::cout << "Invalid entry, please try again." << endl;
        return;
    }
}


void run_program() {
    // Print title
    std::cout << "CSPB 1300 Image Processing Application" << endl;
    
    // Get initial input filename from user
    string in_filename;
    std::cout << "Enter input BMP filename: ";
    std::cin >> in_filename;
    std::cout << endl;

    // Print menu initially
    print_menu(in_filename);

    // Prompt user for selection until quit
    string selection;
    bool done = false;
    while (!done)
    {
        // Get selection
        std::cin >> selection;
        
        // Convert selection from string to int
        int selection_int = convert_selection(selection);
        
        // If selection is converted to -1 user pressed Q
        if (selection_int == -1)
        {
            std::cout << "Thank you for using my program!" << endl
                 << "Quitting..." << endl << endl;
            done = true;
        }
        // Otherwise execute selection, print menu after execution
        else
        {
            execute_selection(&in_filename, selection_int);
            print_menu(in_filename);
        }
    }
}

int main()
{
    // Functions to help test process output, uncomment and recompile to run
    // test();
    // write_all_proceses();
    
    run_program();

    return 0;
}