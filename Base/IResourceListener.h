#pragma once

class ResourceBundle;

class IResourceListener
{
public:
	virtual ~IResourceListener() = default;
	virtual void OnResourceBundleLoaded(ResourceBundle* bundle) = 0;
};