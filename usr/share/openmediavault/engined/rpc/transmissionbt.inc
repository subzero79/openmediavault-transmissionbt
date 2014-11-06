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

require_once "openmediavault/error.inc";
require_once "openmediavault/notify.inc";
require_once "openmediavault/object.inc";
require_once "openmediavault/rpc.inc";
require_once "openmediavault/util.inc";
require_once "transmission/ITorrentServer.php";
require_once "transmission/TransmissionTorrentServer.php";

class OMVRpcServiceTransmissionBt extends OMVRpcServiceAbstract
{
    /* Helpers */

    private function getEventMessagePath()
    {
        return "org.openmediavault.services.transmissionbt";
    }

    private function getXPath()
    {
        return "//services/transmissionbt";
    }

    public function getFilesAndLocationsXPath()
    {
        return sprintf("%s/modules/filesandlocations", $this->getXPath());
    }

    public function getBandwidthXPath()
    {
        return sprintf("%s/modules/bandwidth", $this->getXPath());
    }

    public function getQueuingXPath()
    {
        return sprintf("%s/modules/queuing", $this->getXPath());
    }

    public function getPeerXPath()
    {
        return sprintf("%s/modules/peer", $this->getXPath());
    }

    public function getSchedulingXPath()
    {
        return sprintf("%s/modules/scheduling", $this->getXPath());
    }

    /**
     * Get the RPC service name
     */
    public function getName()
    {
        return "TransmissionBt";
    }

    /**
     * Initialize the RPC service
     */
    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("getFilesAndLocations");
        $this->registerMethod("setFilesAndLocations");
        $this->registerMethod("getBandwidth");
        $this->registerMethod("setBandwidth");
        $this->registerMethod("getQueuing");
        $this->registerMethod("setQueuing");
        $this->registerMethod("getPeer");
        $this->registerMethod("setPeer");
        $this->registerMethod("getScheduling");
        $this->registerMethod("setScheduling");

        // Torrent management functions
        $this->registerMethod("serverIsRunning");
        $this->registerMethod("getTorrentList");
        $this->registerMethod("addTorrent");
        $this->registerMethod("uploadTorrent");
        $this->registerMethod("deleteTorrent");
        $this->registerMethod("pauseTorrent");
        $this->registerMethod("resumeTorrent");
        $this->registerMethod("queueMoveTorrent");
    }

    /**
     * Get general settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $object = $xmlConfig->get($this->getXPath());
        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        // Modify result object
        $object["enable"]                         = boolval($object["enable"]);
        $object["pexenabled"]                     = boolval($object["pexenabled"]);
        $object["dhtenabled"]                     = boolval($object["dhtenabled"]);
        $object["encryption"]                     = intval($object["encryption"]);
        $object["rpcport"]                        = intval($object["rpcport"]);
        $object["rpcurl"]                         = trim($object["rpcurl"], "/");
        $object["rpcauthenticationrequired"]      = boolval($object["rpcauthenticationrequired"]);
        $object["blocklistenabled"]               = boolval($object["blocklistenabled"]);
        $object["blocklistsyncenabled"]           = boolval($object["blocklistsyncenabled"]);
        $object["lazy-bitfield-enabled"]          = boolval($object["lazy-bitfield-enabled"]);
        $object["umask"]                          = intval($object["umask"]);
        $object["lpd-enabled"]                    = boolval($object["lpd-enabled"]);
        $object["scrape-paused-torrents-enabled"] = boolval($object["scrape-paused-torrents-enabled"]);
        $object["utp-enabled"]                    = boolval($object["utp-enabled"]);
        $object["message-level"]                  = intval($object["message-level"]);
        $object["cache-size-mb"]                  = intval($object["cache-size-mb"]);
        $object["script-torrent-done-enabled"]    = boolval($object["script-torrent-done-enabled"]);
        unset($object["modules"]);

        return $object;
    }

    /**
     * Set general settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     * @throws E_CONFIG_SET_OBJECT_FAILED
     * @throws E_CONFIG_SAVE_FAILED
     */
    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "enable"                         : { "type" : "boolean"},
                    "encryption"                     : { "type" : "integer", "enum": [ 0, 1, 2 ] },
                    "message-level"                  : { "type" : "integer", "enum" :[ 0, 1, 2, 3 ] },
                    "lazy-bitfield-enabled"          : { "type" : "boolean" },
                    "scrape-paused-torrents-enabled" : { "type" : "boolean" },
                    "umask"                          : { "type" : "integer" },
                    "cache-size-mb"                  : { "type" : "integer" },
                    "rpcport"                        : { "type" : "integer", "minimum" : 1024, "maximum" : 65535 },
                    "rpcurl"                         : {
                        "type" : "string",
                        "format" : "regex",
                        "pattern" : "/^[a-z0-9]+$/i"
                    },
                    "rpcauthenticationrequired"      : { "type" : "boolean" },
                    "rpcusername"                    : { "type" : "string" },
                    "rpcpassword"                    : { "type" : "string" },
                    "blocklistsyncfrequency"         : {
                        "type" : "string",
                        "enum" : [
                            "hourly",
                            "daily",
                            "weekly",
                            "monthly"
                        ]
                    },
                    "blocklisturl"                   : { "type" : "string" },
                    "script-torrent-done-filename"   : { "type" : "string" },
                    "pexenabled"                     : { "type" : "boolean" },
                    "dhtenabled"                     : { "type" : "boolean" },
                    "lpd-enabled"                    : { "type" : "boolean" },
                    "utp-enabled"                    : { "type" : "boolean" },
                    "blocklistenabled"               : { "type" : "boolean" },
                    "blocklistsyncenabled"           : { "type" : "boolean" },
                    "script-torrent-done-enabled"    : { "type" : "boolean" }
                }
            }'
        );

        // Get existing configuration object
        $oldObject = $xmlConfig->get($this->getXPath());

        if (is_null($oldObject)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $this->getXPath());
        }

        // If no directory for downloads is set
        // and the user is trying to enable the
        // service, abort the save.
        $downloadFolderRef = $oldObject["modules"]
                                       ["filesandlocations"]
                                       ["download.sharedfolderref"];
        $msg = "No directory for downloads set, please set one before enabling the service";

        if ($params["enable"] === true && empty($downloadFolderRef)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED, $msg);
        }

        // Prepare configuration data
        $object = array(
            "enable"                         => array_boolval($params, "enable"),
            "pexenabled"                     => array_boolval($params, "pexenabled"),
            "dhtenabled"                     => array_boolval($params, "dhtenabled"),
            "encryption"                     => $params["encryption"],
            "rpcurl"                         => "/" . $params["rpcurl"] . "/",
            "rpcport"                        => $params["rpcport"],
            "rpcauthenticationrequired"      => array_boolval($params, "rpcauthenticationrequired"),
            "rpcusername"                    => $params["rpcusername"],
            "rpcpassword"                    => $params["rpcpassword"],
            "blocklistenabled"               => array_boolval($params, "blocklistenabled"),
            "blocklistsyncenabled"           => array_boolval($params, "blocklistsyncenabled"),
            "blocklistsyncfrequency"         => $params["blocklistsyncfrequency"],
            "blocklisturl"                   => $params["blocklisturl"],
            "modules"                        => isset($oldObject["modules"]) ? $oldObject["modules"] : array(),
            "lazy-bitfield-enabled"          => array_boolval($params, "lazy-bitfield-enabled"),
            "umask"                          => $params["umask"],
            "lpd-enabled"                    => array_boolval($params, "lpd-enabled"),
            "scrape-paused-torrents-enabled" => array_boolval($params, "scrape-paused-torrents-enabled"),
            "utp-enabled"                    => array_boolval($params, "utp-enabled"),
            "message-level"                  => $params["message-level"],
            "cache-size-mb"                  => $params["cache-size-mb"],
            "script-torrent-done-enabled"    => array_boolval($params, "script-torrent-done-enabled"),
            "script-torrent-done-filename"   => $params["script-torrent-done-filename"],
        );

        // Set configuration object
        if ($xmlConfig->replace($this->getXPath(), $object) === false) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
        }

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, $this->getEventMessagePath(), $object);

        return $object;
    }

    /**
     * Get locations and files settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getFilesAndLocations($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $object = $xmlConfig->get($this->getFilesAndLocationsXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getFilesAndLocationsXPath()
            );
        }

        // Modify result data
        // Locations
        $object["download.sharedfolderref"] = $object["download.sharedfolderref"];
        $object["download-dir"]             = $object["download-dir"];

        $object["incomplete-dir-enabled"]     = boolval($object["incomplete-dir-enabled"]);
        $object["incomplete.sharedfolderref"] = $object["incomplete.sharedfolderref"];
        $object["incomplete-dir"]             = $object["incomplete-dir"];

        $object["watch-dir-enabled"]     = boolval($object["watch-dir-enabled"]);
        $object["watch.sharedfolderref"] = $object["watch.sharedfolderref"];
        $object["watch-dir"]             = $object["watch-dir"];

        // Files
        $object["preallocation"]                = intval($object["preallocation"]);
        $object["rename-partial-files"]         = boolval($object["rename-partial-files"]);
        $object["start-added-torrents"]         = boolval($object["start-added-torrents"]);
        $object["trash-original-torrent-files"] = boolval($object["trash-original-torrent-files"]);

        return $object;
    }

    /**
     * Set locations and files settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setFilesAndLocations($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "download.sharedfolderref"     : {' . $GLOBALS["OMV_JSONSCHEMA_UUID_NONE"] . '},
                    "download-dir"                 : { "type" : "string" },
                    "incomplete-dir-enabled"       : { "type" : "boolean" },
                    "incomplete.sharedfolderref"   : {' . $GLOBALS["OMV_JSONSCHEMA_UUID_NONE"] . '},
                    "incomplete-dir"               : { "type" : "string" },
                    "watch-dir-enabled"            : { "type" : "boolean" },
                    "watch.sharedfolderref"        : {' . $GLOBALS["OMV_JSONSCHEMA_UUID_NONE"] . '},
                    "watch-dir"                    : { "type" : "string" },
                    "preallocation"                : { "type" : "integer", "enum" : [ 0, 1, 2 ] },
                    "rename-partial-files"         : { "type" : "boolean" },
                    "start-added-torrents"         : { "type" : "boolean" },
                    "trash-original-torrent-files" : { "type" : "boolean" }
                }
            }'
        );

        // Prepare configuration data
        $object = array(
            // Locations
            "download.sharedfolderref"   => $params["download.sharedfolderref"],
            "download-dir"               => $params["download-dir"],
            "incomplete-dir-enabled"     => array_boolval($params, "incomplete-dir-enabled"),
            "incomplete.sharedfolderref" => $params["incomplete.sharedfolderref"],
            "incomplete-dir"             => $params["incomplete-dir"],
            "watch-dir-enabled"          => array_boolval($params, "watch-dir-enabled"),
            "watch.sharedfolderref"      => $params["watch.sharedfolderref"],
            "watch-dir"                  => $params["watch-dir"],

            // Files
            "preallocation"                => $params["preallocation"],
            "rename-partial-files"         => array_boolval($params, "rename-partial-files"),
            "start-added-torrents"         => array_boolval($params, "start-added-torrents"),
            "trash-original-torrent-files" => array_boolval($params, "trash-original-torrent-files"),
        );

        // Set configuration object
        if ($xmlConfig->replace($this->getFilesAndLocationsXPath(), $object) === false) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
        }

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, $this->getEventMessagePath(), $object);

        return $object;
    }

    /**
     * Get bandwidth settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getBandwidth($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $object = $xmlConfig->get($this->getBandwidthXPath());
        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getBandwidthXPath()
            );
        }

        // Modify result data
        // Speed
        $object["speed-limit-down"]         = intval($object["speed-limit-down"]);
        $object["speed-limit-down-enabled"] = boolval($object["speed-limit-down-enabled"]);
        $object["speed-limit-up"]           = intval($object["speed-limit-up"]);
        $object["speed-limit-up-enabled"]   = boolval($object["speed-limit-up-enabled"]);
        $object["upload-slots-per-torrent"] = intval($object["upload-slots-per-torrent"]);

        // Turtle Mode
        $object["alt-speed-enabled"] = boolval($object["alt-speed-enabled"]);
        $object["alt-speed-up"]      = intval($object["alt-speed-up"]);
        $object["alt-speed-down"]    = intval($object["alt-speed-down"]);

        return $object;
    }

    /**
     * Set bandwidth settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setBandwidth($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "speed-limit-down"         :{ "type" : "integer", "minimum" : 0 },
                    "speed-limit-down-enabled" :{ "type" : "boolean" },
                    "speed-limit-up"           :{ "type" : "integer", "minimum" : 0 },
                    "speed-limit-up-enabled"   :{ "type" : "boolean" },
                    "upload-slots-per-torrent" :{ "type" : "integer" },
                    "alt-speed-enabled"        :{ "type" : "boolean" },
                    "alt-speed-up"             :{ "type" : "integer", "minimum" : 0 },
                    "alt-speed-down"           :{ "type" : "integer", "minimum" : 0 }
                }
            }'
        );

        // Prepare configuration data
        $object = array(
            // Speed
            "speed-limit-down"         => $params["speed-limit-down"],
            "speed-limit-down-enabled" => array_boolval($params, "speed-limit-down-enabled"),
            "speed-limit-up"           => $params["speed-limit-up"],
            "speed-limit-up-enabled"   => array_boolval($params, "speed-limit-up-enabled"),
            "upload-slots-per-torrent" => $params["upload-slots-per-torrent"],

            // Turtle Mode
            "alt-speed-enabled" => array_boolval($params, "alt-speed-enabled"),
            "alt-speed-up"      => $params["alt-speed-up"],
            "alt-speed-down"    => $params["alt-speed-down"],
        );

        // Set configuration object
        if ($xmlConfig->replace($this->getBandwidthXPath(), $object) === false) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
        }

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, $this->getEventMessagePath(), $object);

        return $object;
    }

    /**
     * Get queuing settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getQueuing($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $object = $xmlConfig->get($this->getQueuingXPath());

        if (is_null($object)) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $this->getQueuingXPath());
        }

        // Modify result data
        // General
        $object["queue-stalled-enabled"] = boolval($object["queue-stalled-enabled"]);
        $object["queue-stalled-minutes"] = intval($object["queue-stalled-minutes"]);

        // Download
        $object["download-queue-enabled"] = boolval($object["download-queue-enabled"]);
        $object["download-queue-size"]    = intval($object["download-queue-size"]);

        // Seed
        $object["seed-queue-enabled"] = boolval($object["seed-queue-enabled"]);
        $object["seed-queue-size"]    = intval($object["seed-queue-size"]);

        return $object;
    }

    /**
     * Set queueing settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setQueuing($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "queue-stalled-enabled"  :{ "type" : "boolean" },
                    "queue-stalled-minutes"  :{ "type" : "integer", "minimum" : 0 },
                    "download-queue-enabled" :{ "type" : "boolean" },
                    "download-queue-size"    :{ "type" : "integer", "minimum" : 0 },
                    "seed-queue-enabled"     :{ "type" : "boolean" },
                    "seed-queue-size"        :{ "type" : "integer", "minimum" : 0 }
                }
            }'
        );

        // Prepare configuration data
        $object = array(
            // General
            "queue-stalled-enabled" => array_boolval($params, "queue-stalled-enabled"),
            "queue-stalled-minutes" => $params["queue-stalled-minutes"],

            // Download
            "download-queue-enabled" => array_boolval($params, "download-queue-enabled"),
            "download-queue-size"    => $params["download-queue-size"],

            // Seed
            "seed-queue-enabled" => array_boolval($params, "seed-queue-enabled"),
            "seed-queue-size"    => $params["seed-queue-size"],
        );

        // Set configuration object
        if ($xmlConfig->replace($this->getQueuingXPath(), $object) === false) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
        }

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, $this->getEventMessagePath(), $object);

        return $object;
    }

    /**
     * Get peer settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getPeer($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $object = $xmlConfig->get($this->getPeerXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getPeerXPath()
            );
        }

        // Modify result data
        // Limits
        $object["peer-limit-global"]      = intval($object["peer-limit-global"]);
        $object["peer-limit-per-torrent"] = intval($object["peer-limit-per-torrent"]);

        // Peer ports
        $object["peer-port"]                 = intval($object["peer-port"]);
        $object["peer-port-random-high"]     = intval($object["peer-port-random-high"]);
        $object["peer-port-random-low"]      = intval($object["peer-port-random-low"]);
        $object["peer-port-random-on-start"] = boolval($object["peer-port-random-on-start"]);
        $object["port-forwarding-enabled"]   = boolval($object["port-forwarding-enabled"]);

        return $object;
    }

    /**
     * Set peer settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setPeer($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "bind-address-ipv4" : {
                        "type" : "string",
                        "format" : "regex",
                        "pattern" : "/^[0-9]{1,3}\\\.[0-9]{1,3}\\\.[0-9]{1,3}\\\.[0-9]{1,3}/"
                    },
                    "bind-address-ipv6"      : { "type" : "string" },
                    "peer-limit-global"      : { "type" : "integer", "minimum" : 0 },
                    "peer-limit-per-torrent" : { "type" : "integer", "minimum" : 0 },
                    "peer-socket-tos"        : {
                        "type" : "string",
                        "enum" : [
                            "default",
                            "lowcost",
                            "throughput",
                            "lowdelay",
                            "reliability"
                        ]
                    },
                    "peer-port"                 : { "type" : "integer", "minimum" : 1024 , "maximum" : 65535 },
                    "peer-port-random-low"      : { "type" : "integer", "minimum" : 1024 , "maximum" : 65535 },
                    "peer-port-random-high"     : { "type" : "integer", "minimum" : 1024 , "maximum" : 65535 },
                    "peer-port-random-on-start" : { "type" : "boolean" },
                    "port-forwarding-enabled"   : { "type" : "boolean" }
                }
            }'
        );

        // Prepare configuration data
        $object = array(
            // Bindings
            "bind-address-ipv4" => $params["bind-address-ipv4"],
            "bind-address-ipv6" => $params["bind-address-ipv6"],

            // Limits
            "peer-limit-global"      => $params["peer-limit-global"],
            "peer-limit-per-torrent" => $params["peer-limit-per-torrent"],
            "peer-socket-tos"        => $params["peer-socket-tos"],

            // Peer ports
            "peer-port"                 => $params["peer-port"],
            "peer-port-random-high"     => $params["peer-port-random-high"],
            "peer-port-random-low"      => $params["peer-port-random-low"],
            "peer-port-random-on-start" => array_boolval($params, "peer-port-random-on-start"),
            "port-forwarding-enabled"   => array_boolval($params, "port-forwarding-enabled"),
        );

        // Set configuration object
        if ($xmlConfig->replace($this->getPeerXPath(), $object) === false) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
        }

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            $object
        );

        return $object;
    }

    /**
     * Get scheduling settings object
     *
     * @throws E_CONFIG_GET_OBJECT_FAILED
     */
    public function getScheduling($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $object = $xmlConfig->get($this->getSchedulingXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getSchedulingXPath()
            );
        }

        // Modify result data
        // Scheduling
        $object["alt-speed-time-enabled"] = boolval($object["alt-speed-time-enabled"]);
        $object["alt-speed-time-begin"]   = intval($object["alt-speed-time-begin"]);
        $object["begin-hour"]             = intval($object["alt-speed-time-begin"] / 60);
        $object["begin-minute"]           = ($object["alt-speed-time-begin"] % 60);
        $object["alt-speed-time-end"]     = intval($object["alt-speed-time-end"]);
        $object["end-hour"]               = intval($object["alt-speed-time-end"] / 60);
        $object["end-minute"]             = ($object["alt-speed-time-end"] % 60);

        // Days
        $object["days-sunday"] = false;
        if (($object["alt-speed-time-day"] & 1) === 1) {
            $object["days-sunday"] = true;
        }
        $object["days-monday"] = false;
        if (($object["alt-speed-time-day"] & 2) === 2) {
            $object["days-monday"] = true;
        }
        $object["days-tuesday"] = false;
        if (($object["alt-speed-time-day"] & 4) === 4) {
            $object["days-tuesday"] = true;
        }
        $object["days-wednesday"] = false;
        if (($object["alt-speed-time-day"] & 8) === 8) {
            $object["days-wednesday"] = true;
        }
        $object["days-thursday"] = false;
        if (($object["alt-speed-time-day"] & 16) === 16) {
            $object["days-thursday"] = true;
        }
        $object["days-friday"] = false;
        if (($object["alt-speed-time-day"] & 32) === 32) {
            $object["days-friday"] = true;
        }
        $object["days-saturday"] = false;
        if (($object["alt-speed-time-day"] & 64) === 64) {
            $object["days-saturday"] = true;
        }

        // Idle
        $object["idle-seeding-limit"]         = intval($object["idle-seeding-limit"]);
        $object["idle-seeding-limit-enabled"] = boolval($object["idle-seeding-limit-enabled"]);

        // Ratio
        $object["ratio-limit"]         = floatval($object["ratio-limit"]);
        $object["ratio-limit-enabled"] = boolval($object["ratio-limit-enabled"]);

        return $object;
    }

    /**
     * Set scheduling settings object
     *
     * @throws E_CONFIG_SET_OBJECT_FAILED
     */
    public function setScheduling($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "alt-speed-time-enabled"     : { "type" : "boolean" },
                    "begin-hour"                 : { "type" : "integer", "minimum" : 0, "maximum" : 23 },
                    "begin-minute"               : { "type" : "integer", "minimum" : 0, "maximum" : 59 },
                    "end-hour"                   : { "type" : "integer", "minimum" : 0, "maximum" : 23 },
                    "end-minute"                 : { "type" : "integer", "minimum" : 0, "maximum" : 59 },
                    "days-sunday"                : { "type" : "boolean" },
                    "days-monday"                : { "type" : "boolean" },
                    "days-tuesday"               : { "type" : "boolean" },
                    "days-wednesday"             : { "type" : "boolean" },
                    "days-thursday"              : { "type" : "boolean" },
                    "days-friday"                : { "type" : "boolean" },
                    "days-saturday"              : { "type" : "boolean" },
                    "idle-seeding-limit"         : { "type" : "integer", "minimum" : 0 },
                    "idle-seeding-limit-enabled" : { "type" : "boolean" },
                    "ratio-limit"                : { "type" : "number", "minimum" : 0 },
                    "ratio-limit-enabled"        : { "type" : "boolean" }
                }
            }'
        );

        $params["alt-speed-time-day"] = 0;

        if (array_boolval($params, "days-sunday") === true) {
            $params["alt-speed-time-day"] += 1;
        }
        if (array_boolval($params, "days-monday") === true) {
            $params["alt-speed-time-day"] += 2;
        }
        if (array_boolval($params, "days-tuesday") === true) {
            $params["alt-speed-time-day"] += 4;
        }
        if (array_boolval($params, "days-wednesday") === true) {
            $params["alt-speed-time-day"] += 8;
        }
        if (array_boolval($params, "days-thursday") === true) {
            $params["alt-speed-time-day"] += 16;
        }
        if (array_boolval($params, "days-friday") === true) {
            $params["alt-speed-time-day"] += 32;
        }
        if (array_boolval($params, "days-saturday") === true) {
            $params["alt-speed-time-day"] += 64;
        }

        // Prepare configuration data
        $object = array(
            "alt-speed-time-enabled" => array_boolval($params, "alt-speed-time-enabled"),
            "alt-speed-time-begin"   => intval(($params["begin-hour"] * 60) + $params["begin-minute"]),
            "alt-speed-time-end"     => intval(($params["end-hour"] * 60) + $params["end-minute"]),
            "alt-speed-time-day"     => $params["alt-speed-time-day"],

            // Idle
            "idle-seeding-limit"         => $params["idle-seeding-limit"],
            "idle-seeding-limit-enabled" => array_boolval($params, "idle-seeding-limit-enabled"),

            // Ratio
            "ratio-limit"         => $params["ratio-limit"],
            "ratio-limit-enabled" => array_boolval($params, "ratio-limit-enabled"),
        );

        // Set configuration object
        if ($xmlConfig->replace($this->getSchedulingXPath(), $object) === false) {
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
        }

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, $this->getEventMessagePath(), $object);

        return $object;
    }

    /* Torrent management section */

    public function serverIsRunning()
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        $module = new OMVModuleTransmissionBt();
        $status = $module->getStatus();

        return $status["running"];
    }

    public function getTorrentList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "start"     : { "type" : "integer" },
                    "limit"     : {' . $GLOBALS["OMV_JSONSCHEMA_COUNTFIELD"] . '},
                    "sortfield" : {' . $GLOBALS["OMV_JSONSCHEMA_SORTFIELD"] . '},
                    "sortdir"   : {' . $GLOBALS["OMV_JSONSCHEMA_SORTDIR"] . '}
                }
            }'
        );

        $server = $this->torrentServerFactory();
        $torrents = array();

        foreach ($server->getTorrents() as $torrent) {
            $torrents[] = array(
                "id"                      => $torrent->getId(),
                "name"                    => $torrent->getName(),
                "status"                  => $torrent->getStatus(),
                "total_size"              => $torrent->getTotalSize(),
                "have_valid"              => $torrent->getHaveValid(),
                "percent_done"            => $torrent->getPercentDownloaded(),
                "eta"                     => $torrent->getEta(),
                "connected_peers"         => $torrent->getConnectedPeers(),
                "connected_peers_sending" => $torrent->getConnectedPeersSending(),
                "download_rate"           => $torrent->getDownloadSpeed(),
                "upload_rate"             => $torrent->getUploadSpeed(),
                "date_added"              => $torrent->getDateAdded(),
                "date_done"               => $torrent->getDateDone(),
                "upload_ratio"            => $torrent->getRatio(),
                "queue_position"          => $torrent->getQueuePosition()
            );
        }

        return $this->applyFilter(
            $torrents,
            $params["start"],
            $params["limit"],
            $params["sortfield"],
            $params["sortdir"]
        );
    }

    public function addTorrent($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"      : "object",
                "properties" : {
                    "url"            : { "type" : "string" },
                    "start_download" : { "type" : "boolean" }
                }
            }'
        );

        $server = $this->torrentServerFactory();

        $url = $params["url"];
        $paused = !array_boolval($params, "start_download");

        return $server->add($url, $paused);
    }

    public function uploadTorrent($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "filename" : { "type" : "string" },
                    "filepath" : { "type" : "string" }
                }
            }'
        );

        $server = $this->torrentServerFactory();

        return $server->add($params["filepath"]);
    }

    public function deleteTorrent($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "id"                : { "type" : "integer", "minimum" : 0 },
                    "delete_local_data" : { "type" : "boolean" }
                }
            }'
        );

        $server = $this->torrentServerFactory();

        $torrent = $params["id"];
        $deleteLocalData = array_boolval($params, "delete_local_data");

        $server->delete($torrent, $deleteLocalData);
    }

    public function pauseTorrent($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "id" : { "type" : "integer", "minimum" : 0 }
                }
            }'
        );

        $server = $this->torrentServerFactory();
        $server->pause($params["id"]);
    }

    public function resumeTorrent($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "id" : { "type" : "integer", "minimum" : 0 }
                }
            }'
        );

        $server = $this->torrentServerFactory();
        $server->resume($params["id"]);
    }

    public function queueMoveTorrent($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "id"     : { "type" : "integer", "minimum" : 0 },
                    "action" : { "type" : "string", "enum" : [ "top", "up", "down", "bottom" ] }
                }
            }'
        );

        $server = $this->torrentServerFactory();

        switch ($params["action"]) {
            case "top":
            case "up":
            case "down":
            case "bottom":
                $server->queueMove($params["id"], $params["action"]);
                break;
            default:
                throw new OMVException("Unknown Action: " . $params["action"]);
                break;
        }
    }

    /**
     * Returns a ITorrentServer class instance
     *
     * @return ITorrentServer
     */
    private function torrentServerFactory()
    {
        global $xmlConfig;

        $object = $xmlConfig->get($this->getXPath());
        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        $url = "http://localhost:" . $object["rpcport"] . $object["rpcurl"] . "rpc";
        $username;
        $password;

        if (array_boolval($object, "rpcauthenticationrequired") === true) {
            $username = $object["rpcusername"];
            $password = $object["rpcpassword"];
        }

        $server = new TransmissionTorrentServer();
        $server->connect($url, $username, $password);

        return $server;
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceTransmissionBt());
