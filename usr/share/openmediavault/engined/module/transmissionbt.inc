<?php

/**
 * Copyright (C) 2009-2012 Volker Theile <volker.theile@openmediavault.org>
 * Copyright (C) 2011-2012 Marcel Beck <marcel.beck@mbeck.org>
 * Copyright (C) 2013-2015 OpenMediaVault Plugin Developers
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
    OMVIModuleServiceStatus,
    OMVIModuleZeroconf,
    OMVINotifyListener
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
        return "/config/services/transmissionbt";
    }

    /**
     * Get the module name.
     *
     * @return string
     */
    public function getName()
    {
        return "transmissionbt";
    }

    /**
     * Generate the configuration.
     *
     * @return void
     */
    public function applyConfig()
    {
        $cmd = sprintf("export LANG=C; omv-mkconf %s 2>&1", $this->getName());

        if (0 !== $this->exec($cmd, $output)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode("\n", $output)
            );
        }
    }

    /**
     * Start the managed service.
     *
     * @return void
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
     *
     * @return void
     */
    public function stopService()
    {
        $initd = new OMVSysVInitScript("transmission-daemon");
        $initd->exec();
    }

    /**
     * Get the status of the managed service.
     *
     * @return array
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
            "name" => $this->getName(),
            "title" => gettext("BitTorrent"),
            "enabled" => boolval($object['enable']),
            "running" => !empty($output[0])
        );
    }

    /**
     * Get the Zeroconf service configuration.
     *
     * @return array
     */
    public function getZeroconfConfig()
    {
        return array(
            "id" => "transmissionbt",
            "title" => gettext("BitTorrent"),
        );
    }

    /**
     * Bind listeners.
     *
     * @param OMVNotifyDispatcher $dispatcher The dispatcher object.
     *
     * @return void
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

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleTransmissionBt());
