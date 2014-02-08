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

require_once "transmission/apis/TransmissionRPC.php"; 

class OMVTransmissionRpc extends TransmissionRPC
{
    /**
     * Move torrents in the queue
     *
     * @param int|array ids A list of transmission torrent ids
     * @param string action The move action to perform
     * @return void
     */
    public function queueMove($ids, $action)
    {
        switch ($action) {
            case "top":
            case "up":
            case "down":
            case "bottom":
                break;
            default:
                throw new \Exception("$action is not a valid action.");
                break;
        }

        if (!is_array($ids))
            $ids = array($ids);
        
        // Convert $ids to an array if only a single id was passed
        $request = array("ids" => $ids);
        $this->request("queue-move-$action", $request);
    }
}
