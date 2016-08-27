

DROP TABLE IF EXISTS `account`;

CREATE TABLE `account` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'Identifier',
  `username` VARCHAR(32) NOT NULL DEFAULT '',
  `sha_pass_hash` VARCHAR(40) NOT NULL DEFAULT '',
  `sessionkey` VARCHAR(80) NOT NULL DEFAULT '',
  `v` VARCHAR(64) NOT NULL DEFAULT '',
  `s` VARCHAR(64) NOT NULL DEFAULT '',
  `token_key` VARCHAR(100) NOT NULL DEFAULT '',
  `email` VARCHAR(254) NOT NULL DEFAULT '',
  `joindate` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_ip` VARCHAR(15) NOT NULL DEFAULT '127.0.0.1',
  `failed_logins` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `locked` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `last_login` TIMESTAMP NOT NULL DEFAULT '0000-00-00 00:00:00',
  `online` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `expansion` TINYINT(3) UNSIGNED NOT NULL DEFAULT '4',
  `mutetime` BIGINT(20) NOT NULL DEFAULT '0',
  `locale` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `os` VARCHAR(3) NOT NULL DEFAULT '',
  `recruiter` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_username` (`username`)
) ENGINE=INNODB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COMMENT='Account System';

/*Table structure for table `account_access` */

DROP TABLE IF EXISTS `account_access`;

CREATE TABLE `account_access` (
  `id` INT(10) UNSIGNED NOT NULL,
  `gmlevel` TINYINT(3) UNSIGNED NOT NULL,
  `RealmID` INT(11) NOT NULL DEFAULT '-1',
  `comment` TEXT,
  PRIMARY KEY (`id`,`RealmID`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

/*Data for the table `account_access` */

/*Table structure for table `account_banned` */

DROP TABLE IF EXISTS `account_banned`;

CREATE TABLE `account_banned` (
  `id` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Account id',
  `bandate` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `unbandate` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `bannedby` VARCHAR(50) NOT NULL,
  `banreason` VARCHAR(255) NOT NULL,
  `active` TINYINT(3) UNSIGNED NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`,`bandate`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COMMENT='Ban List';

/*Data for the table `account_banned` */

/*Table structure for table `account_log_ip` */

DROP TABLE IF EXISTS `account_log_ip`;

CREATE TABLE `account_log_ip` (
  `accountid` INT(11) UNSIGNED NOT NULL,
  `ip` VARCHAR(30) NOT NULL DEFAULT '0.0.0.0',
  `date` DATETIME DEFAULT NULL,
  PRIMARY KEY (`accountid`,`ip`)
) ENGINE=MYISAM DEFAULT CHARSET=latin1;

/*Data for the table `account_log_ip` */

INSERT  INTO `account_log_ip`(`accountid`,`ip`,`date`) VALUES (1,'127.0.0.1','2015-08-21 23:31:01');

/*Table structure for table `account_premium` */

DROP TABLE IF EXISTS `account_premium`;

CREATE TABLE `account_premium` (
  `id` INT(11) NOT NULL DEFAULT '0' COMMENT 'Account id',
  `setdate` BIGINT(40) NOT NULL DEFAULT '0',
  `unsetdate` BIGINT(40) NOT NULL DEFAULT '0',
  `premium_type` TINYINT(4) UNSIGNED NOT NULL DEFAULT '1',
  `gm` VARCHAR(12) NOT NULL DEFAULT '0',
  `active` TINYINT(4) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`,`setdate`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

/*Data for the table `account_premium` */

/*Table structure for table `account_spell` */

DROP TABLE IF EXISTS `account_spell`;

CREATE TABLE `account_spell` (
  `accountId` INT(11) NOT NULL,
  `spell` INT(10) NOT NULL,
  `active` TINYINT(1) DEFAULT NULL,
  `disabled` TINYINT(1) DEFAULT NULL,
  PRIMARY KEY (`accountId`,`spell`),
  KEY `account` (`accountId`) USING HASH,
  KEY `account_spell` (`accountId`,`spell`) USING HASH
) ENGINE=INNODB DEFAULT CHARSET=latin1;

/*Data for the table `account_spell` */

/*Table structure for table `firewall_farms` */

DROP TABLE IF EXISTS `firewall_farms`;

CREATE TABLE `firewall_farms` (
  `ip` TINYTEXT NOT NULL,
  `type` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=INNODB DEFAULT CHARSET=latin1;

/*Data for the table `firewall_farms` */

/*Table structure for table `ip_banned` */

DROP TABLE IF EXISTS `ip_banned`;

CREATE TABLE `ip_banned` (
  `ip` VARCHAR(15) NOT NULL DEFAULT '127.0.0.1',
  `bandate` INT(10) UNSIGNED NOT NULL,
  `unbandate` INT(10) UNSIGNED NOT NULL,
  `bannedby` VARCHAR(50) NOT NULL DEFAULT '[Console]',
  `banreason` VARCHAR(255) NOT NULL DEFAULT 'no reason',
  PRIMARY KEY (`ip`,`bandate`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COMMENT='Banned IPs';

/*Data for the table `ip_banned` */

/*Table structure for table `ip_to_country` */

DROP TABLE IF EXISTS `ip_to_country`;

CREATE TABLE `ip_to_country` (
  `IP_FROM` DOUBLE NOT NULL,
  `IP_TO` DOUBLE NOT NULL,
  `COUNTRY_CODE` CHAR(2) DEFAULT NULL,
  `COUNTRY_CODE_3` CHAR(3) NOT NULL,
  `COUNTRY_NAME` VARCHAR(50) DEFAULT NULL,
  PRIMARY KEY (`IP_FROM`,`IP_TO`)
) ENGINE=MYISAM DEFAULT CHARSET=latin1;

/*Data for the table `ip_to_country` */

/*Table structure for table `log_vote` */

DROP TABLE IF EXISTS `log_vote`;

CREATE TABLE `log_vote` (
  `top_name` VARCHAR(15) NOT NULL DEFAULT 'top',
  `ip` VARCHAR(15) NOT NULL DEFAULT '0.0.0.0',
  `date` INT(11) NOT NULL,
  PRIMARY KEY (`top_name`,`ip`)
) ENGINE=INNODB DEFAULT CHARSET=latin1;

/*Data for the table `log_vote` */

/*Table structure for table `logs` */

DROP TABLE IF EXISTS `logs`;

CREATE TABLE `logs` (
  `time` INT(10) UNSIGNED NOT NULL,
  `realm` INT(10) UNSIGNED NOT NULL,
  `type` TINYINT(3) UNSIGNED NOT NULL,
  `level` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `string` TEXT CHARACTER SET latin1
) ENGINE=INNODB DEFAULT CHARSET=utf8;

/*Data for the table `logs` */

/*Table structure for table `mails` */

DROP TABLE IF EXISTS `mails`;

CREATE TABLE `mails` (
  `email` VARCHAR(254) NOT NULL DEFAULT ''
) ENGINE=INNODB DEFAULT CHARSET=utf8;

/*Data for the table `mails` */

/*Table structure for table `realm_classes` */

DROP TABLE IF EXISTS `realm_classes`;

CREATE TABLE `realm_classes` (
  `realmId` INT(11) NOT NULL,
  `class` TINYINT(4) NOT NULL COMMENT 'Class Id',
  `expansion` TINYINT(4) NOT NULL COMMENT 'Expansion for class activation',
  PRIMARY KEY (`realmId`,`class`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

/*Data for the table `realm_classes` */

INSERT  INTO `realm_classes`(`realmId`,`class`,`expansion`) VALUES (1,1,0),(1,2,0),(1,3,0),(1,4,0),(1,5,0),(1,6,2),(1,7,0),(1,8,0),(1,9,0),(1,10,4),(1,11,0);

/*Table structure for table `realm_races` */

DROP TABLE IF EXISTS `realm_races`;

CREATE TABLE `realm_races` (
  `realmId` INT(11) NOT NULL,
  `race` TINYINT(4) NOT NULL COMMENT 'Race Id',
  `expansion` TINYINT(4) NOT NULL COMMENT 'Expansion for race activation',
  PRIMARY KEY (`realmId`,`race`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

/*Data for the table `realm_races` */

INSERT  INTO `realm_races`(`realmId`,`race`,`expansion`) VALUES (1,1,0),(1,2,0),(1,3,0),(1,4,0),(1,5,0),(1,6,0),(1,7,0),(1,8,0),(1,9,3),(1,10,1),(1,11,1),(1,22,3),(1,24,4),(1,25,4),(1,26,4);

/*Table structure for table `realmcharacters` */

DROP TABLE IF EXISTS `realmcharacters`;

CREATE TABLE `realmcharacters` (
  `realmid` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `acctid` INT(10) UNSIGNED NOT NULL,
  `numchars` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`realmid`,`acctid`),
  KEY `acctid` (`acctid`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COMMENT='Realm Character Tracker';

/*Data for the table `realmcharacters` */

INSERT  INTO `realmcharacters`(`realmid`,`acctid`,`numchars`) VALUES (1,1,4),(2,1,0);

/*Table structure for table `realmlist` */

DROP TABLE IF EXISTS `realmlist`;

CREATE TABLE `realmlist` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(32) NOT NULL DEFAULT 'MoPCore Server',
  `address` VARCHAR(255) NOT NULL DEFAULT '127.0.0.1',
  `port` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '8085',
  `icon` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `flag` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `timezone` TINYINT(3) UNSIGNED NOT NULL DEFAULT '1',
  `allowedSecurityLevel` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `population` FLOAT UNSIGNED NOT NULL DEFAULT '0',
  `gamebuild` INT(10) UNSIGNED NOT NULL DEFAULT '18414',
  `online` INT(10) DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_name` (`name`)
) ENGINE=INNODB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8 COMMENT='Realm System';

/*Data for the table `realmlist` */

INSERT  INTO `realmlist`(`id`,`name`,`address`,`port`,`icon`,`flag`,`timezone`,`allowedSecurityLevel`,`population`,`gamebuild`,`online`) VALUES (1,'MoPCore Server','127.0.0.1',8085,0,0,1,0,0,18019,0);

/*Table structure for table `transferts` */

DROP TABLE IF EXISTS `transferts`;

CREATE TABLE `transferts` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `account` INT(11) NOT NULL,
  `perso_guid` INT(11) NOT NULL,
  `from` INT(11) NOT NULL,
  `to` INT(11) NOT NULL,
  `revision` BLOB NOT NULL,
  `dump` LONGTEXT NOT NULL,
  `last_error` BLOB NOT NULL,
  `nb_attempt` INT(11) NOT NULL,
  `state` INT(10) DEFAULT NULL,
  `error` INT(10) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=INNODB DEFAULT CHARSET=latin1;

/*Data for the table `transferts` */

/*Table structure for table `transferts_logs` */

DROP TABLE IF EXISTS `transferts_logs`;

CREATE TABLE `transferts_logs` (
  `id` INT(11) DEFAULT NULL,
  `account` INT(11) DEFAULT NULL,
  `perso_guid` INT(11) DEFAULT NULL,
  `from` INT(2) DEFAULT NULL,
  `to` INT(2) DEFAULT NULL,
  `dump` LONGTEXT
) ENGINE=INNODB DEFAULT CHARSET=latin1;

/*Data for the table `transferts_logs` */

/*Table structure for table `uptime` */

DROP TABLE IF EXISTS `uptime`;

CREATE TABLE `uptime` (
  `realmid` INT(10) UNSIGNED NOT NULL,
  `starttime` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `uptime` INT(10) UNSIGNED NOT NULL DEFAULT '0',
  `maxplayers` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
  `revision` VARCHAR(255) NOT NULL DEFAULT 'Trinitycore',
  PRIMARY KEY (`realmid`,`starttime`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COMMENT='Uptime system';

/*Data for the table `uptime` */

INSERT  INTO `uptime`(`realmid`,`starttime`,`uptime`,`maxplayers`,`revision`) VALUES (1,1440186787,600,0,'WoWSource rev.  () (Win64, Release)'),(1,1440188885,0,0,'WoWSource rev.  () (Win64, Release)'),(1,1440189197,1800,1,'WoWSource rev.  () (Win64, Release)'),(1,1440191920,0,0,'WoWSource rev.  () (Win64, Release)');


