
// QT includes
#include <QApplication>
#include <QImage>

#include <commandline/Parser.h>
#include <flatbufserver/FlatBufferConnection.h>
#include <utils/DefaultSignalHandler.h>
#include "XcbWrapper.h"
#include "HyperionConfig.h"

// ssdp discover
#include <ssdp/SSDPDiscover.h>

using namespace commandline;

// save the image as screenshot
void saveScreenshot(QString filename, const Image<ColorRgb> & image)
{
	// store as PNG
	QImage pngImage((const uint8_t *) image.memptr(), image.width(), image.height(), 3*image.width(), QImage::Format_RGB888);
	pngImage.save(filename);
}

int main(int argc, char ** argv)
{
	Logger *log = Logger::getInstance("XCBGRABBER");
	Logger::setLogLevel(Logger::INFO);

	 std::cout
		<< "hyperion-xcb:" << std::endl
		<< "\tVersion   : " << HYPERION_VERSION << " (" << HYPERION_BUILD_ID << ")" << std::endl
		<< "\tbuild time: " << __DATE__ << " " << __TIME__ << std::endl;

	DefaultSignalHandler::install();

	QApplication app(argc, argv);
	try
	{
		// create the option parser and initialize all parameters
		Parser parser("XCB capture application for Hyperion. Will automatically search a Hyperion server if -a option isn't used. Please note that if you have more than one server running it's more or less random which one will be used.");

		IntOption           & argFps             = parser.add<IntOption>    ('f', "framerate", "Capture frame rate [default: %1]", "10");
		IntOption           & argCropWidth       = parser.add<IntOption>    (0x0, "crop-width", "Number of pixels to crop from the left and right sides of the picture before decimation [default: %1]", "0");
		IntOption           & argCropHeight      = parser.add<IntOption>    (0x0, "crop-height", "Number of pixels to crop from the top and the bottom of the picture before decimation [default: %1]", "0");
		IntOption           & argCropLeft        = parser.add<IntOption>    (0x0, "crop-left", "Number of pixels to crop from the left of the picture before decimation (overrides --crop-width)");
		IntOption           & argCropRight       = parser.add<IntOption>    (0x0, "crop-right", "Number of pixels to crop from the right of the picture before decimation (overrides --crop-width)");
		IntOption           & argCropTop         = parser.add<IntOption>    (0x0, "crop-top", "Number of pixels to crop from the top of the picture before decimation (overrides --crop-height)");
		IntOption           & argCropBottom      = parser.add<IntOption>    (0x0, "crop-bottom", "Number of pixels to crop from the bottom of the picture before decimation (overrides --crop-height)");
		IntOption           & argSizeDecimation  = parser.add<IntOption>    ('s', "size-decimator", "Decimation factor for the output size [default=%1]", "8", 1);
		BooleanOption       & argScreenshot      = parser.add<BooleanOption>(0x0, "screenshot", "Take a single screenshot, save it to file and quit");
		Option              & argAddress         = parser.add<Option>       ('a', "address", "Set the address of the hyperion server [default: %1]", "127.0.0.1:19400");
		IntOption           & argPriority        = parser.add<IntOption>    ('p', "priority", "Use the provided priority channel (suggested 100-199) [default: %1]", "150");
		BooleanOption       & argSkipReply       = parser.add<BooleanOption>(0x0, "skip-reply", "Do not receive and check reply messages from Hyperion");
		BooleanOption       & argDebug           = parser.add<BooleanOption>(0x0, "debug", "Enable debug logging");
		BooleanOption       & argHelp            = parser.add<BooleanOption>('h', "help", "Show this help message and exit");

		// parse all options
		parser.process(app);

		// check if debug logging is required
		if (parser.isSet(argDebug))
		{
			Logger::setLogLevel(Logger::DEBUG);
		}

		// check if we need to display the usage. exit if we do.
		if (parser.isSet(argHelp))
		{
			parser.showHelp(0);
		}

		// Create the XCB grabbing stuff
		XcbWrapper xcbWrapper(
					1000 / argFps.getInt(parser),
					parser.isSet(argCropLeft) ? argCropLeft.getInt(parser) : argCropWidth.getInt(parser),
					parser.isSet(argCropRight) ? argCropRight.getInt(parser) : argCropWidth.getInt(parser),
					parser.isSet(argCropTop) ? argCropTop.getInt(parser) : argCropHeight.getInt(parser),
					parser.isSet(argCropBottom) ? argCropBottom.getInt(parser) : argCropHeight.getInt(parser),
					argSizeDecimation.getInt(parser)); // decimation

		if (!xcbWrapper.displayInit())
			return -1;

		if (parser.isSet(argScreenshot))
		{
			// Capture a single screenshot and finish
			const Image<ColorRgb> & screenshot = xcbWrapper.getScreenshot();
			saveScreenshot("screenshot.png", screenshot);
		}
		else
		{
			// server searching by ssdp
			QString address = argAddress.value(parser);
			if(argAddress.value(parser) == "127.0.0.1:19400")
			{
				SSDPDiscover discover;
				address = discover.getFirstService(searchType::STY_FLATBUFSERVER);
				if(address.isEmpty())
				{
					address = argAddress.value(parser);
				}
			}
			// Create the Flatbuf-connection
			FlatBufferConnection flatbuf("XCB Standalone", address, argPriority.getInt(parser), parser.isSet(argSkipReply));

			// Connect the screen capturing to flatbuf connection processing
			QObject::connect(&xcbWrapper, SIGNAL(sig_screenshot(const Image<ColorRgb> &)), &flatbuf, SLOT(setImage(Image<ColorRgb>)));

			// Start the capturing
			xcbWrapper.start();

			// Start the application
			app.exec();
		}
	}
	catch (const std::runtime_error & e)
	{
		// An error occurred. Display error and quit
		Error(log, "%s", e.what());
		return -1;
	}

	return 0;
}
