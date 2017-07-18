CREATE DEFINER=`root`@`%` PROCEDURE `edubot_inst_anim`()
BEGIN

DECLARE i int DEFAULT -1;
    WHILE i <= 63 DO
        INSERT INTO `edubot`.`animation` (`id`,`type`, `duration`, `repeat`, `interpolate`) VALUES (i,i, '1', '1', '0');
        SET i = i + 1;
    END WHILE;

END