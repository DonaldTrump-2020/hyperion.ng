#pragma once

#include <QObject>

// DirectX 9 header
#include <d3d9.h>
#include <d3dx9.h>

// Hyperion-utils includes
#include <utils/ColorRgb.h>
#include <hyperion/Grabber.h>

// qt includes
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

///
/// @brief The DirectX9 capture implementation
///
class DirectXGrabber : public Grabber
{
public:

	DirectXGrabber(int cropLeft = 0, int cropRight = 0, int cropTop = 0, int cropBottom = 0, int pixelDecimation = 8, int display = 8);

	virtual ~DirectXGrabber();

	///
	/// Captures a single snapshot of the display and writes the data to the given image. The
	/// provided image should have the same dimensions as the configured values (_width and
	/// _height)
	///
	/// @param[out] image  The snapped screenshot
	///
	virtual int grabFrame(Image<ColorRgb> & image);

	///
	/// @brief Set a new video mode
	///
	virtual void setVideoMode(VideoMode mode);

	///
	/// @brief Apply new width/height values, overwrite Grabber.h implementation
	///
	virtual bool setWidthHeight(int width, int height) { return true; };

	///
	/// @brief Apply new pixelDecimation
	///
	virtual bool setPixelDecimation(int pixelDecimation)  override;

	///
	/// Set the crop values
	/// @param  cropLeft    Left pixel crop
	/// @param  cropRight   Right pixel crop
	/// @param  cropTop     Top pixel crop
	/// @param  cropBottom  Bottom pixel crop
	///
	virtual void setCropping(unsigned cropLeft, unsigned cropRight, unsigned cropTop, unsigned cropBottom);

	///
	/// @brief Apply display index
	///
	void setDisplayIndex(int index) override;

	/// @brief Discover QT screens available (for configuration).
	///
	/// @param[in] params Parameters used to overwrite discovery default behaviour
	///
	/// @return A JSON structure holding a list of devices found
	///
	QJsonObject discover(const QJsonObject& params);

private:
	///
	/// @brief Setup a new capture display, will free the previous one
	/// @return True on success, false if no display is found
	///
	bool setupDisplay();

	///
	/// @brief free the _screen pointer
	///
	void freeResources();

private:
	unsigned _display;
	unsigned _displayWidth;
	unsigned _displayHeight;
	RECT* _srcRect;

	IDirect3D9* _d3d9;
	IDirect3DDevice9* _device;
	IDirect3DSurface9* _surface;
	IDirect3DSurface9* _surfaceDest;
};
