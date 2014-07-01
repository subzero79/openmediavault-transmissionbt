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

require_once "transmission/ITorrent.php";

class TransmissionTorrent implements ITorrent
{
    private $torrent;

    public function __construct($data)
    {
        $this->torrent = $data;
    }

    public function getId()
    {
        return $this->torrent["id"];
    }

    public function getName()
    {
        return $this->torrent["name"];
    }

    public function getStatus()
    {
        return $this->torrent["status"];
    }

    public function getTotalSize()
    {
        return $this->torrent["totalSize"];
    }

    public function getHaveValid()
    {
        return $this->torrent["haveValid"];
    }

    public function getPercentDownloaded()
    {
        return $this->torrent["percentDone"];
    }

    public function getEta()
    {
        return $this->torrent["eta"];
    }

    public function getConnectedPeers()
    {
        return $this->torrent["peersConnected"];
    }

    public function getConnectedPeersSending()
    {
        return $this->torrent["peersSendingToUs"];
    }

    public function getDownloadSpeed()
    {
        return $this->torrent["rateDownload"];
    }

    public function getUploadSpeed()
    {
        return $this->torrent["rateUpload"];
    }

    public function getDateAdded()
    {
        return $this->torrent["addedDate"];
    }

    public function getDateDone()
    {
        return $this->torrent["doneDate"];
    }

    public function getRatio()
    {
        return $this->torrent["uploadRatio"];
    }

    public function getQueuePosition()
    {
        return $this->torrent["queuePosition"];
    }
}
