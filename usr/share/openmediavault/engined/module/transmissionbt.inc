<?php

/**
 * Copyright (C) 2009-2012 Volker Theile <volker.theile@openmediavault.org>
 * Copyright (C) 2011-2012 Marcel Beck <marcel.beck@mbeck.org>
 * Copyright (C) 2013-2014 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/initscript.inc";
require_once "openmediavault/module.inc";
require_once "openmediavault/util.inc";

class OMVModuleTransmissionBt extends OMVModuleServiceAbstract implements
    OMVINotifyListener,
    OMVIModuleServiceStatus
{
    private function getEventMessagePath()
    {
        return "org.openmediavault.services.transmissionbt";
    }

    private function getSharedFolderEventMessagePath()
    {
        return "org.openmediavault.system.shares.sharedfolder";
    }

    private function getXPath()
    {
        return "//services/transmissionbt";
    }

    /**
     * Get the module name.
     */
    public function getName()
    {
        return "transmissionbt";
    }

    /**
     * Generate the configuration.
     */
    public function applyConfig()
    {
        // Currently transmission overwrites the settings.json configuration
        // file on exit. This means transmission must be terminated before
        // writing the new settings.json file.
        $this->stopService();

        $cmd = sprintf("omv-mkconf %s 2>&1", $this->getName());

        if (0 !== $this->exec($cmd, $output)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode("\n", $output)
            );
        }
    }

    /**
     * Start the managed service
     */
    public function startService()
    {
        global $xmlConfig;

        // Get the configuration object
        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        if (!boolval($object['enable'])) {
            return;
        }

        $initd = new OMVSysVInitScript("transmission-daemon", true);
        $initd->exec();
    }

    /**
     * Stop the managed service.
     */
    public function stopService()
    {
        $initd = new OMVSysVInitScript("transmission-daemon");
        $initd->exec();
    }

    /**
     * Get the status of the managed service.
     */
    public function getStatus()
    {
        global $xmlConfig;

        // Get the configuration object
        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        // Check if process is running. Init script does not support 'status'.
        $cmd = "ps --no-heading -C transmission-daemon";
        $this->exec($cmd, $output);

        return array(
            "name"    => $this->getName(),
            "title"   => gettext("BitTorrent"),
            "enabled" => boolval($object['enable']),
            "running" => !empty($output[0])
        );
    }

    /**
     * Bind listeners.
     */
    public function bindListeners(OMVNotifyDispatcher $dispatcher)
    {
        $moduleMgr = &OMVModuleMgr::getInstance();

        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            array($this, "setDirty")
        );

        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            array($moduleMgr->getModule("zeroconf"), "setDirty")
        );

        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getSharedFolderEventMessagePath(),
            array($this, "setDirty")
        );
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleTransmissionBt());

// Register new logfile type.
OMVLogFile::registerType(
    "transmissionbt",
    array(
        "filename" => "transmissionbt",
        "filepath" => "/var/log/transmissionbt.log",
        "regex"    => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+transmission-daemon\[\d+\]:\s+(.*)$/",
        "columns"  => array(
            "ts" => array(
                "index" => 1,
                "func" => create_function(
                    "\$v",
                    "return strpdate(\$v, \"M j G:i:s\");"
                )
            ),
            "date" => array(
                "index" => 1,
                "func"  => create_function(
                    "\$v",
                    "return strftime(\"%c\", strpdate(\$v, \"M j G:i:s\"));"
                )
            ),
            "event" => 3
        )
    )
);
