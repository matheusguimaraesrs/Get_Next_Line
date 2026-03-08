#include "get_next_line.h"

static char	*first_line(char *line)
{
	char	*line_cuted;
	int		position;

	position = (ft_strchr(line, '\n') - line) + 1;
	line_cuted = ft_substr(line, 0, position);
	free(line);
	return (line_cuted);
}

static char	*get_rest(char *line)
{
	char	*rest;
	int		position;

	position = (ft_strchr(line, '\n') - line) + 1;
	rest = ft_substr(line, position, ft_strlen(line));
	if (*rest == '\0')
	{
		free(rest);
		rest = NULL;
		return (rest);
	}
	return (rest);
}

static char	*read_file(int fd, char *rest, char *buffer)
{
	char	*temp;
	ssize_t	readed;

	while (1)
	{
		readed = read(fd, buffer, BUFFER_SIZE);
		if (readed == -1)
		{
			free(rest);
			return (NULL);
		}
		else if (readed == '\0')
			break ;
		buffer[readed] = '\0';
		if (rest == NULL)
			rest = ft_strdup("");
		temp = rest;
		rest = ft_strjoin(temp, buffer);
		free(temp);
		temp = NULL;
		if (ft_strchr(rest, '\n'))
			break ;
	}
	return (rest);
}

char	*get_next_line(int fd)
{
	static char	*rest;
	char		*line;
	char		*buffer;

	if (fd < 0 || BUFFER_SIZE < 1)
		return (NULL);
	buffer = (char *)malloc((BUFFER_SIZE + 1) * sizeof(char));
	if (!buffer)
		return (NULL);
	line = read_file(fd, rest, buffer);
	free(buffer);
	buffer = NULL;
	if (!line)
		return (NULL);
	rest = get_rest(line);
	line = first_line(line);
	return (line);
}

////////////////////////////////////////////////////////////////////
//                           MAIN                                //
//////////////////////////////////////////////////////////////////
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>

int	main(void)
{
	int		fd;
	char	*line;

	fd = open("text.txt", O_RDONLY);
	line = 0;
	while ((line = get_next_line(fd)))
	{
		printf("%s", line);
		free(line);
	}
	close(fd);
	return (0);
}

