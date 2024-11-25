﻿/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io

* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "integrationplugingenericcar.h"
#include "plugininfo.h"

#include <QDebug>
#include <QtMath>

IntegrationPluginGenericCar::IntegrationPluginGenericCar()
{

}

void IntegrationPluginGenericCar::setupThing(ThingSetupInfo *info)
{
    Thing *thing = info->thing();

    // Set the min charging current state if the settings value changed
    connect(thing, &Thing::settingChanged, this, [thing](const ParamTypeId &paramTypeId, const QVariant &value){
        qCDebug(dcGenericCar()) << "Setting" << paramTypeId << "changed to" << value;
        if (paramTypeId == carSettingsCapacityParamTypeId) {
            thing->setStateValue(carCapacityStateTypeId, value);
        } else if (paramTypeId == carSettingsMinChargingCurrentParamTypeId) {
            qCDebug(dcGenericCar()) << "Car minimum charging current settings changed" << value.toUInt() << "A";
            thing->setStateValue(carMinChargingCurrentStateTypeId, value);
        } else if (paramTypeId == carSettingsPhaseCountParamTypeId) {
            thing->setStateValue(carPhaseCountStateTypeId, value);
        }
    });

    // Migration from earlier versions (pre 1.3) which had the capacity setting as a writable state (???).
    // This causes the user configured setting to be set to the default value 50 after a reboot.
    // TODO: should likely be removed
    //
    // thing->setSettingValue(carSettingsCapacityParamTypeId, thing->stateValue(carCapacityStateTypeId));

    // Set the inital state value to the configured user settings
    // Takes effect after a restart or reboot. Important to have the correct states after a device reboot.
    thing->setStateValue(carMinChargingCurrentStateTypeId, thing->setting(carSettingsMinChargingCurrentParamTypeId));
    thing->setStateValue(carCapacityStateTypeId, thing->setting(carSettingsCapacityParamTypeId));
    thing->setStateValue(carPhaseCountStateTypeId, thing->setting(carSettingsPhaseCountParamTypeId));

    // Finish the setup
    info->finish(Thing::ThingErrorNoError);
}

void IntegrationPluginGenericCar::executeAction(ThingActionInfo *info)
{
    Thing *thing = info->thing();
    Action action = info->action();

    if (action.actionTypeId() == carBatteryLevelActionTypeId) {
        thing->setStateValue(carBatteryLevelStateTypeId, action.paramValue(carBatteryLevelActionBatteryLevelParamTypeId));
        thing->setStateValue(carBatteryCriticalStateTypeId, action.paramValue(carBatteryLevelActionBatteryLevelParamTypeId).toInt() < 10);
        info->finish(Thing::ThingErrorNoError);
    } else {
        Q_ASSERT_X(false, "executeAction", QString("Unhandled action: %1").arg(action.actionTypeId().toString()).toUtf8());
    }
}
