CREATE TABLE `highscores` (
  `username` varchar(30) COLLATE utf8mb4_unicode_ci NOT NULL,
  `skin` varchar(20) COLLATE utf8mb4_unicode_ci NOT NULL,
  `map_id` int(11) NOT NULL,
  `time` int(11) DEFAULT NULL,
  `time_string` text COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`username`,`skin`,`map_id`),
  KEY `map_id` (`map_id`),
  CONSTRAINT `highscores_ibfk_1` FOREIGN KEY (`map_id`) REFERENCES `maps` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `maps` (
  `id` int(11) NOT NULL,
  `name` text COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
