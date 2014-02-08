<?php

/**
 * Copyright (C) 2014 OpenMediaVault Plugin Developers
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

require_once "transmission/apis/OMVTransmissionRpc.php";
require_once "transmission/ITorrentServer.php";
require_once "transmission/TransmissionTorrent.php";

class TransmissionTorrentServer implements ITorrentServer
{
    private $_rpc;

    public static function getDefaultServerAddress()
    {
        return "http://localhost:9091/transmission/rpc";
    }

    public function connect($url, $username, $password)
    {
        $this->_rpc = new OMVTransmissionRpc($url, $username, $password, true);
    }

    public function getTorrents()
    {
        $result = $this->_rpc->get(
            array(),
            array(
                "id",
                "name",
                "status",
                "doneDate",
                "addedDate",
                "haveValid",
                "totalSize",
                "percentDone",
                "eta",
                "peersConnected",
                "peersSendingToUs",
                "rateDownload",
                "rateUpload",
                "uploadRatio",
                "queuePosition"
            )
        );

        $torrents = array();

        foreach ($result["arguments"]["torrents"] as $torrent) {
            $torrents[] = new TransmissionTorrent($torrent);
        }

        return $torrents;
    }

    public function add($location, $paused = false)
    {
        $result = $this->_rpc->add($location, "", array("paused" => $paused));

        if (isset($result["result"])) {
            if ($result["result"] == "success")
                return true;

            throw new Exception($result["result"]);
        }

        throw new Exception("Something unexpected went wrong when adding the torrent");
    }

    public function delete($torrent, $deleteLocalData)
    {
        $this->_rpc->remove($torrent, $deleteLocalData);
    }

    public function pause($torrent)
    {
        $this->_rpc->stop($torrent);
    }

    public function resume($torrent)
    {
        $this->_rpc->start($torrent);
    }
    
    public function queueMove($torrent, $action)
    {
        $this->_rpc->queueMove($torrent, $action);
    }

}
