/****************************************************************************
 *
 *   (c) 2017 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#include "AirspaceManager.h"
#include "AirspaceWeatherInfoProvider.h"
#include "AirspaceAdvisoryProvider.h"
#include "AirspaceRestriction.h"
#include "AirspaceRestrictionProvider.h"
#include "AirspaceRulesetsProvider.h"
#include "AirspaceVehicleManager.h"
#include "AirspaceController.h"

#include "Vehicle.h"
#include "QGCApplication.h"

QGC_LOGGING_CATEGORY(AirspaceManagementLog, "AirspaceManagementLog")

AirspaceManager::AirspaceManager(QGCApplication* app, QGCToolbox* toolbox)
    : QGCTool(app, toolbox)
{
    _roiUpdateTimer.setInterval(2000);
    _roiUpdateTimer.setSingleShot(true);
    connect(&_roiUpdateTimer, &QTimer::timeout, this, &AirspaceManager::_updateToROI);
    qmlRegisterUncreatableType<AirspaceAuthorization>       ("QGroundControl",              1, 0, "AirspaceAuthorization",      "Reference only");
    qmlRegisterUncreatableType<AirspaceController>          ("QGroundControl.Vehicle",      1, 0, "AirspaceController",         "Reference only");
    qmlRegisterUncreatableType<AirspaceWeatherInfoProvider> ("QGroundControl.Vehicle",      1, 0, "AirspaceWeatherInfoProvider","Reference only");
    qmlRegisterUncreatableType<AirspaceAdvisoryProvider>    ("QGroundControl.Vehicle",      1, 0, "AirspaceAdvisoryProvider",   "Reference only");
    qmlRegisterUncreatableType<AirspaceRule>                ("QGroundControl.Vehicle",      1, 0, "AirspaceRule",               "Reference only");
    qmlRegisterUncreatableType<AirspaceRulesetsProvider>    ("QGroundControl.Vehicle",      1, 0, "AirspaceRulesetsProvider",   "Reference only");
}

AirspaceManager::~AirspaceManager()
{
    if(_advisories) {
        delete _advisories;
    }
    if(_weatherProvider) {
        delete _weatherProvider;
    }
    if(_rulesetsProvider) {
        delete _rulesetsProvider;
    }
    if (_restrictionsProvider) {
        delete _restrictionsProvider;
    }
    _polygonRestrictions.clearAndDeleteContents();
    _circleRestrictions.clearAndDeleteContents();
}

void AirspaceManager::setToolbox(QGCToolbox* toolbox)
{
    QGCTool::setToolbox(toolbox);
    // We should not call virtual methods in the constructor, so we instantiate the restriction provider here
    _restrictionsProvider = instantiateRestrictionProvider();
    if (_restrictionsProvider) {
        connect(_restrictionsProvider, &AirspaceRestrictionProvider::requestDone, this,   &AirspaceManager::_restrictionsUpdated);
    }
    _rulesetsProvider   = instantiateRulesetsProvider();
    _weatherProvider    = instatiateAirspaceWeatherInfoProvider();
    _advisories         = instatiateAirspaceAdvisoryProvider();
}

void AirspaceManager::setROI(const QGeoCoordinate& center, double radiusMeters)
{
    _roiCenter = center;
    _roiRadius = radiusMeters;
    _roiUpdateTimer.start();
}

void AirspaceManager::_updateToROI()
{
    /*
    if (_restrictionsProvider) {
        _restrictionsProvider->setROI(_roiCenter, _roiRadius);
    }
    if(_rulesetsProvider) {
        _rulesetsProvider->setROI(_roiCenter);
    }
    */
    if(_weatherProvider) {
        _weatherProvider->setROI(_roiCenter);
    }
    if (_advisories) {
        _advisories->setROI(_roiCenter, _roiRadius);
    }
}

void AirspaceManager::_restrictionsUpdated(bool success)
{
    _polygonRestrictions.clearAndDeleteContents();
    _circleRestrictions.clearAndDeleteContents();
    if (success) {
        for (const auto& circle : _restrictionsProvider->circles()) {
            _circleRestrictions.append(circle);
        }
        for (const auto& polygon : _restrictionsProvider->polygons()) {
            _polygonRestrictions.append(polygon);
        }
    } else {
        // TODO: show error?
    }
}

void AirspaceManager::_rulessetsUpdated(bool)
{

}
