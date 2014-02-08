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

interface ITorrent
{
    public function __construct($data);

    /**
     * Get torrent id
     *
     * @return mixed
     */
    public function getId();

    /**
     * Get the torrent name
     *
     * @return string
     */
    public function getName();

    /**
     * Get the torrent status
     *
     * 0: "Torrent is stopped";
     * 1: "Queued to check files"
     * 2: "Checking files"
     * 3: "Queued to download"
     * 4: "Downloading"
     * 5: "Queued to seed"
     * 6: "Seeding"
     * default: "Missing Status"
     *
     * @return string
     */
    public function getStatus();

    /**
     * Get the torrent total size in bytes
     *
     * @return int
     */
    public function getTotalSize();

    /**
     * Get the torrent valid downloaded size in bytes
     *
     * @return int
     */
    public function getHaveValid();

    /**
     * Get the downloaded percentage
     *
     * @return int
     */
    public function getPercentDownloaded();

    /**
     * Get the downloaded percentage
     *
     * -1: "Not available"
     * -2: "Unknown"
     *
     * @return mixed
     */
    public function getEta();

    /**
     * Get the connected peers count
     *
     * @return int
     */
    public function getConnectedPeers();

    /**
     * Get the connected peers sending to us count
     *
     * @return int
     */
    public function getConnectedPeersSending();

    /**
     * Get the download speed in bytes
     *
     * @return int
     */
    public function getDownloadSpeed();

    /**
     * Get the upload speed in bytes
     *
     * @return int
     */
    public function getUploadSpeed();

    /**
     * Get the date the torrent was added
     *
     * @return string
     */
    public function getDateAdded();

    /**
     * Get the date the torrent was completed
     *
     * @return string
     */
    public function getDateDone();

    /**
     * Get the upload ratio of the torrent. Return -1 if unavailable, and -2 if infinite.
     *
     * @return float|integer
     */
    public function getRatio();

    /**
     * Get the position of the torrent in the queue
     *
     * @return int
     */
    public function getQueuePosition();
}
